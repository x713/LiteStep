# LiteStep Architecture

This document provides a high-level overview of the LiteStep architecture. The application is divided into several distinct layers, each with its own set of responsibilities.

## Layers

The LiteStep codebase is organized into four main layers:

1.  **Foundation Layer (`utility`)**: This is the lowest level of the architecture. It provides a set of common utilities and abstractions that are used by all other layers. This includes:
    -   Core data structures and macros.
    -   Wrappers for the Windows API.
    -   Interfaces for services and managers.
    -   Helpers for thread safety, memory management, and string manipulation.

2.  **API Layer (`lsapi`)**: This layer exposes the LiteStep API to external modules. It provides a stable interface that allows developers to extend LiteStep's functionality. The key features of the API layer include:
    -   **Bang Commands**: A system for creating and managing custom commands.
    -   **Settings Management**: A way to read and write configuration files.
    -   **Math Library**: A component for parsing and evaluating mathematical expressions.
    -   **Graphics and UI**: Functions for drawing, loading images, and creating UI elements.

3.  **Application Layer (`litestep`)**: This is the core of the LiteStep application. It is responsible for:
    -   Managing the lifecycle of LiteStep modules.
    -   Integrating with the Windows shell (e.g., appbar, system tray).
    -   Handling system messages and events.
    -   Providing core services like data storage and inter-process communication (DDE).

4.  **System Hook Layer (`hook`)**: This is a small, specialized layer that is responsible for low-level system hooking. It is implemented as a separate DLL (`hook.dll`) that is injected into other processes to intercept system events.

## Interactions

The layers interact in a hierarchical manner:

-   The **Application Layer** uses the **API Layer** to provide services to modules and the **Foundation Layer** for common utilities.
-   The **API Layer** uses the **Foundation Layer** for its implementation.
-   The **System Hook Layer** is used by the **Application Layer** to monitor system activity.

This layered architecture promotes a separation of concerns and makes the codebase easier to understand, maintain, and extend.
