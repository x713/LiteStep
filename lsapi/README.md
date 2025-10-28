# LiteStep API (LSAPI)

The LiteStep API (LSAPI) is a dynamic-link library (`lsapi.dll`) that serves as the primary interface between the LiteStep core and its modules. It provides a comprehensive set of functions that allow modules to interact with the shell, manage their own configuration, and create user interface elements.

## Key Features

The LSAPI is organized into several key areas of functionality:

### Configuration Management

The LSAPI provides a set of functions for reading and writing configuration settings. The primary functions in this area are:

-   `LCOpen`, `LCClose`, `LCReadNextCommand`, `LCReadNextConfig`, `LCReadNextLine`: These functions provide low-level access to the `step.rc` file and other configuration sources.
-   `GetRCInt`, `GetRCString`, `GetRCBool`, `GetRCColor`: These functions provide a simpler, more direct way to read specific types of values from the configuration files.
-   `LSGetVariable`, `LSSetVariable`: These functions allow modules to create and use their own dynamic variables.

### Bang Commands

The LSAPI provides a powerful "bang command" system that allows modules to register and execute simple, text-based commands. The key functions for this system are:

-   `AddBangCommand`, `RemoveBangCommand`: These functions allow modules to register and unregister their own bang commands.
-   `ParseBangCommand`: This function allows modules to execute bang commands.

### Graphics and UI

The LSAPI provides a set of helper functions for creating and managing user interface elements. These functions include:

-   `BitmapToRegion`: Creates a window region from a bitmap, allowing for non-rectangular windows.
-   `LoadLSImage`: Loads an image from a file, with support for PNG images.
-   `TransparentBltLS`: Draws a bitmap with transparency.
-   `Frame3D`: Draws a 3D-style frame.

### System Integration

The LSAPI provides a set of functions for interacting with the underlying Windows operating system. These functions include:

-   `LSExecute`: Executes a command or launches an application.
-   `GetLitestepWnd`: Returns the handle to the main LiteStep window.
-   `LSGetLitestepPath`, `LSGetImagePath`: These functions return the paths to the LiteStep installation and image directories, respectively.
-   `LSGetSystemMetrics`, `LSMonitorFromWindow`, `LSGetMonitorInfo`: These functions provide access to system metrics and information about the display monitors.

## PNG Image Support

The LSAPI provides support for loading PNG images through the `LoadLSImage` function. This functionality is implemented using the `libpng` library, which is a popular open-source library for working with PNG files.

### Replacing `libpng` with WIC

While `libpng` is a reliable and widely used library, it is also a third-party dependency that needs to be separately compiled and maintained. For modern Windows development, a better alternative is the Windows Imaging Component (WIC), which is a built-in component of the Windows operating system.

WIC provides a modern, COM-based API for working with a wide variety of image formats, including PNG, JPEG, BMP, and more. By replacing `libpng` with WIC, we can:

-   **Reduce external dependencies:** WIC is a part of the operating system, so there is no need to compile and link a separate library.
-   **Improve performance:** WIC is a highly optimized, low-level API that can provide better performance than `libpng` in many cases.
-   **Support more image formats:** WIC supports a wide variety of image formats out of the box, which could allow us to easily add support for other image types in the future.
