# LiteStep Architecture

The LiteStep shell is built upon a modular, layered architecture that allows for extensive customization and extensibility. This document provides an overview of the key components and their interactions.

## Core Components

The LiteStep architecture consists of three primary layers:

1.  **The Core (litestep.exe):** The central component of the shell, responsible for initializing the environment, managing the lifecycle of modules, and providing essential system-level services.
2.  **The LiteStep API (lsapi.dll):** A dynamic-link library that serves as the intermediary between the core and the modules. It exposes a set of functions that modules can use to interact with the core and other modules.
3.  **Modules (\*.dll):** Pluggable components that provide the actual features and functionality of the shell, such as a taskbar, system tray, or desktop icons.

## Architectural Layers

### 1. The Core (litestep.exe)

The core is the foundation of the LiteStep shell. Its main responsibilities include:

-   **Initialization:** The core is responsible for initializing the shell, setting up the main window, and preparing the environment for loading modules.
-   **Module Management:** The `ModuleManager` is a key component of the core, responsible for loading, unloading, and managing the lifecycle of all modules. It reads the `step.rc` configuration file to determine which modules to load at startup.
-   **System Integration:** The core handles low-level system integration tasks, such as registering as the system shell and managing the desktop window.
-   **Service Provision:** The core provides a set of services, such as DDE and tray icon management, that can be used by modules.

### 2. The LiteStep API (lsapi.dll)

The LSAPI is the glue that connects the core and the modules. It provides a stable and consistent interface for modules to interact with the shell, without needing to know the internal details of the core. The LSAPI's key functions include:

-   **Configuration Management:** The LSAPI provides a set of functions for reading and writing configuration settings from the `step.rc` file and other configuration sources.
-   **Bang Command System:** The LSAPI manages the registration and execution of "bang commands," which are simple text-based commands that can be used to trigger actions within the shell.
-   **Graphics and UI:** The LSAPI provides a set of helper functions for creating and managing UI elements, such as windows, menus, and icons. It also includes support for loading and rendering images, including PNG files.
-   **System Information:** The LSAPI provides functions for retrieving system information, such as the current screen resolution and the location of system folders.

### 3. Modules

Modules are the building blocks of the LiteStep shell. They are responsible for providing all of the user-facing features and functionality. Each module is a separate DLL that is loaded by the core at startup. Modules use the LSAPI to:

-   **Create UI Elements:** Modules can create windows, menus, and other UI elements to provide a user interface for their features.
-   **Register Bang Commands:** Modules can register their own bang commands to allow users to interact with them via the command line or other modules.
-   **Access Configuration:** Modules can use the LSAPI to read their own configuration settings from the `step.rc` file.
-   **Interact with the System:** Modules can use the LSAPI to interact with the system in a variety of ways, such as launching applications or manipulating windows.

## Modernization Suggestions

While the current LiteStep architecture is flexible and extensible, there are several areas where it could be modernized to better support modern Windows operating systems.

### 64-bit Compatibility

The codebase already includes a 64-bit build configuration, which is a great starting point. However, a thorough review of the code should be conducted to identify and address any potential issues related to 64-bit compatibility, such as:

-   **Pointer precision:** Ensure that all code that deals with pointers is using the correct data types (e.g., `uintptr_t` or `size_t`) to avoid truncation issues.
-   **Data alignment:** Verify that all data structures are properly aligned to avoid performance penalties and potential crashes.
-   **API usage:** Ensure that all Windows API calls are using the correct, 64-bit-compatible versions.

### C++ Modernization

The codebase currently uses a mix of C and C++ styles. A concerted effort should be made to modernize the code by:

-   **Using C++ features:** Replace legacy C-style code with modern C++ features, such as smart pointers, `std::vector`, and `std::string`.
-   **Adopting a consistent style:** Establish and enforce a consistent coding style throughout the codebase to improve readability and maintainability.
-   **Refactoring large functions:** Break down large, complex functions into smaller, more manageable pieces to improve modularity and testability.

### High-DPI Support

Modern displays often have high resolutions and DPI settings, which can cause older applications to appear small and difficult to use. To improve the user experience on these displays, the following changes should be made:

-   **DPI-aware UI:** The UI code should be updated to be DPI-aware, so that it can scale correctly on high-DPI displays.
-   **Vector-based icons:** Consider replacing the current bitmap-based icons with vector-based icons, which can scale to any size without losing quality.
-   **Dynamic layout:** The UI layout should be made more dynamic, so that it can adapt to different screen sizes and resolutions.
