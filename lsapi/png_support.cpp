#include "png_support.h" // Should be the first include
#include "lodepng.h"       // For LodePNG functions
#include <windows.h>       // For HBITMAP, BITMAPINFO, GetObject, DeleteObject, MultiByteToWideChar, DeleteFileA etc.
#include <vector>          // For std::vector
#include <fstream>         // For std::ifstream
#include <algorithm>       // For std::swap
#include <string>          // For std::string
#include <iostream>        // For std::cout in RunPNGLoadTest

// Make sure common.h (via png_support.h) or other headers provide
// necessary types like LPCWSTR if not already included by windows.h

// Renamed from LoadFromPNG
HBITMAP lspng_load_file(LPCWSTR pwszFilename) {
    std::vector<unsigned char> png_file_buffer;

    std::ifstream file_stream(pwszFilename, std::ios::binary);
    if (!file_stream.is_open()) {
        return NULL;
    }

    file_stream.seekg(0, std::ios::end);
    std::streamsize file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    if (file_size <= 0) {
        file_stream.close();
        return NULL;
    }

    png_file_buffer.resize(static_cast<size_t>(file_size));
    if (!file_stream.read(reinterpret_cast<char*>(png_file_buffer.data()), file_size)) {
        file_stream.close();
        return NULL;
    }
    file_stream.close();

    if (png_file_buffer.empty()) {
        return NULL;
    }

    std::vector<unsigned char> image_data;
    unsigned int width, height;

    unsigned error = lodepng::decode(image_data, width, height, png_file_buffer.data(), png_file_buffer.size(), LCT_RGBA, 8);

    if (error) {
        // OutputDebugStringA(lodepng_error_text(error));
        return NULL;
    }

    if (width == 0 || height == 0 || image_data.empty()) {
        return NULL;
    }

    for (size_t i = 0; i < image_data.size(); i += 4) {
        std::swap(image_data[i], image_data[i + 2]); // Swap R and B for BGRA
    }

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    unsigned char* pPixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(
        NULL, &bmi, DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pPixels),
        NULL, 0);

    if (!hBitmap || !pPixels) {
        return NULL;
    }

    int dib_stride = ((width * bmi.bmiHeader.biBitCount + 31) & ~31) >> 3;
    int image_pixel_stride = width * 4;

    for (unsigned int y = 0; y < height; ++y) {
        memcpy(pPixels + (y * dib_stride), image_data.data() + (y * image_pixel_stride), image_pixel_stride);
    }
    
    return hBitmap;
}

bool lspng_save_file(const std::string& filename, const unsigned char* image_data, unsigned int width, unsigned int height, bool input_is_bgra) {
    if (!image_data || width == 0 || height == 0) {
        return false;
    }

    unsigned error;
    if (input_is_bgra) {
        std::vector<unsigned char> rgba_data(width * height * 4);
        for (size_t i = 0; i < width * height * 4; i += 4) {
            rgba_data[i + 0] = image_data[i + 2]; // R
            rgba_data[i + 1] = image_data[i + 1]; // G
            rgba_data[i + 2] = image_data[i + 0]; // B
            rgba_data[i + 3] = image_data[i + 3]; // A
        }
        error = lodepng::encode(filename, rgba_data, width, height, LCT_RGBA, 8);
    } else {
        error = lodepng::encode(filename, image_data, width, height, LCT_RGBA, 8);
    }

    return error == 0;
}

// --- Test-related Function Implementations ---

// Helper function to convert std::string to std::wstring for Windows API calls
#if defined(_WIN32)
static std::wstring ConvertStringToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
#endif

static bool GenerateTestPatternData(std::vector<unsigned char>& image_data, unsigned int width, unsigned int height) {
    if (width == 0 || height == 0) {
        return false;
    }

    image_data.assign(width * height * 4, 255); // Initialize to white (255,255,255,255) RGBA

    // Draw black diagonal cross (RGBA: 0,0,0,255)
    for (unsigned int i = 0; i < width && i < height; ++i) {
        // Top-left to bottom-right
        size_t index1 = (i * width + i) * 4;
        image_data[index1 + 0] = 0;   // R
        image_data[index1 + 1] = 0;   // G
        image_data[index1 + 2] = 0;   // B
        image_data[index1 + 3] = 255; // A

        // Top-right to bottom-left
        if (width > i) { // Ensure x coordinate (width - 1 - i) is valid
             size_t index2 = (i * width + (width - 1 - i)) * 4;
             image_data[index2 + 0] = 0;   // R
             image_data[index2 + 1] = 0;   // G
             image_data[index2 + 2] = 0;   // B
             image_data[index2 + 3] = 255; // A
        }
    }
    return true;
}

static bool CheckPixelBGRA(const unsigned char* pixel_data, unsigned int x, unsigned int y, unsigned int dib_width, 
                           unsigned char b, unsigned char g, unsigned char r, unsigned char a, const std::string& pixel_name) {
    size_t offset = (y * dib_width + x) * 4; 
    bool match = (pixel_data[offset + 0] == b &&
                  pixel_data[offset + 1] == g &&
                  pixel_data[offset + 2] == r &&
                  pixel_data[offset + 3] == a);
    if (!match) {
        std::cout << "FAIL: " << pixel_name << " at (" << x << "," << y << ") - Expected (B:" << (int)b << ",G:" << (int)g << ",R:" << (int)r << ",A:" << (int)a 
                  << ") Got (B:" << (int)pixel_data[offset + 0] << ",G:" << (int)pixel_data[offset + 1] << ",R:" << (int)pixel_data[offset + 2] << ",A:" << (int)pixel_data[offset + 3] << ")" << std::endl;
    } else {
         std::cout << "PASS: " << pixel_name << " at (" << x << "," << y << ")" << std::endl;
    }
    return match;
}

bool RunPNGLoadTest() {
    const std::string test_filename = "test_pattern.png";
    const unsigned int test_width = 100;
    const unsigned int test_height = 100;
    bool all_tests_passed = true;

    std::cout << "Starting PNG Load Test (using lspng_ wrappers)..." << std::endl;

    std::vector<unsigned char> generated_image_data;
    if (!GenerateTestPatternData(generated_image_data, test_width, test_height)) {
        std::cout << "FAIL: GenerateTestPatternData failed." << std::endl;
        return false;
    }
    std::cout << "INFO: Test pattern data generated." << std::endl;

    if (!lspng_save_file(test_filename, generated_image_data.data(), test_width, test_height, false /*is_bgra=false*/)) {
        std::cout << "FAIL: lspng_save_file failed to create " << test_filename << std::endl;
        return false;
    }
    std::cout << "INFO: " << test_filename << " created successfully via lspng_save_file." << std::endl;

#if defined(_WIN32)
    std::wstring wide_test_filename = ConvertStringToWstring(test_filename); 
    HBITMAP hBitmap = lspng_load_file(wide_test_filename.c_str());
#else
    HBITMAP hBitmap = nullptr; 
    std::cout << "WARN: lspng_load_file test is Windows-specific (HBITMAP)." << std::endl;
    all_tests_passed = false; 
#endif

    if (!hBitmap && all_tests_passed) {
        std::cout << "FAIL: lspng_load_file failed to load " << test_filename << std::endl;
        DeleteFileA(test_filename.c_str()); 
        return false;
    }
    if(hBitmap) std::cout << "INFO: " << test_filename << " loaded into HBITMAP via lspng_load_file." << std::endl;

    BITMAP bmp;
    if (hBitmap && GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) {
        std::cout << "FAIL: GetObject failed for HBITMAP." << std::endl;
        all_tests_passed = false;
    }

    if (hBitmap && all_tests_passed) {
        if (static_cast<unsigned int>(bmp.bmWidth) != test_width || static_cast<unsigned int>(abs(bmp.bmHeight)) != test_height) {
            std::cout << "FAIL: Bitmap dimensions mismatch. Got " << bmp.bmWidth << "x" << abs(bmp.bmHeight) 
                      << ", expected " << test_width << "x" << test_height << std::endl;
            all_tests_passed = false;
        } else {
            std::cout << "INFO: Bitmap dimensions match." << std::endl;
        }
        
        if (bmp.bmBitsPixel != 32) {
             std::cout << "FAIL: Expected 32 bits per pixel, got " << bmp.bmBitsPixel << std::endl;
             all_tests_passed = false;
        }

        if (all_tests_passed && bmp.bmBits && bmp.bmWidth > 0 && abs(bmp.bmHeight) > 0) {
            unsigned char* pixel_data_from_dib = static_cast<unsigned char*>(bmp.bmBits);
            unsigned int dib_actual_width = bmp.bmWidth;

            if (!CheckPixelBGRA(pixel_data_from_dib, 0, 0, dib_actual_width, 0,0,0,255, "Top-left (0,0) - BGRA")) all_tests_passed = false;
            if (test_width > 1) {
                if (!CheckPixelBGRA(pixel_data_from_dib, 1, 0, dib_actual_width, 255,255,255,255, "Near top-left (1,0) - BGRA")) all_tests_passed = false;
            }
            if (test_height > 1) {
                 if (!CheckPixelBGRA(pixel_data_from_dib, 0, 1, dib_actual_width, 255,255,255,255, "Near top-left (0,1) - BGRA")) all_tests_passed = false;
            }
            unsigned int cx = test_width / 2;
            unsigned int cy = test_height / 2;
            if (!CheckPixelBGRA(pixel_data_from_dib, cx, cy, dib_actual_width, 0,0,0,255, "Center (cx,cy) - BGRA")) all_tests_passed = false;
            if (test_width > cx + 1) {
                 if (!CheckPixelBGRA(pixel_data_from_dib, cx + 1, cy, dib_actual_width, 255,255,255,255, "Near Center (cx+1,cy) - BGRA")) all_tests_passed = false;
            }
            if (test_width > 0 && test_height > 0) {
                 if (!CheckPixelBGRA(pixel_data_from_dib, test_width - 1, test_height - 1, dib_actual_width, 0,0,0,255, "Bottom-right - BGRA")) all_tests_passed = false;
            }
        } else if (all_tests_passed) {
            std::cout << "FAIL: Could not access pixel data or dimensions were zero for loaded HBITMAP." << std::endl;
            all_tests_passed = false;
        }
    }

    if(hBitmap) DeleteObject(hBitmap);
    if (std::ifstream(test_filename).good()) { // Check if file exists before attempting to delete
        if (!DeleteFileA(test_filename.c_str())) {
            std::cout << "WARN: Failed to delete " << test_filename << " after test." << std::endl;
        }
    }


    if (all_tests_passed) {
        std::cout << "SUCCESS: All PNG wrapper and load checks passed." << std::endl;
    } else {
        std::cout << "FAILURE: Some PNG wrapper and load checks failed." << std::endl;
    }
    
    return all_tests_passed;
}
