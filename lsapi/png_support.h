/*
This is a part of the LiteStep Shell Source code.

Copyright (C) 2025 The x7 Dev Team
Copyright (C) 1997-2002 The LiteStep Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef __PNG_SUPPORT_H
#define __PNG_SUPPORT_H

#include "../utility/common.h" // This line should remain
#include <string>              // For std::string

// common.h or its own includes (like windows.h) should provide HBITMAP and LPCWSTR.
// If common.h is minimal and doesn't include windows.h, then png_support.h
// might need to #include <windows.h> directly if these types are not defined.
// For this task, assume common.h handles it or that necessary Windows types are globally available.


// --- LSAPI PNG Wrapper API ---
// Replaces LoadFromPNG as the primary public function for loading PNGs.
HBITMAP lspng_load_file(LPCWSTR filename);

// Saves raw pixel data to a PNG file.
// image_data: pointer to the raw pixel data.
// input_is_bgra: true if image_data is in BGRA format, false if RGBA.
//                LodePNG expects RGBA for encoding, so conversion might be needed.
bool lspng_save_file(const std::string& filename, const unsigned char* image_data, unsigned int width, unsigned int height, bool input_is_bgra);


// --- Test-related Functions ---
// Declaration for the main test function to be called by the test runner.
bool RunPNGLoadTest(); 

#endif // __PNG_SUPPORT_H
