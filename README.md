# Freeze keyboard layout

I have no idea which application on my system is activating alternative keyboard layouts for my current keyboard layout language. These extra layouts do not show up in Control Panel as added.

This application periodically enumerates all currently loaded keyboard layouts into the system (not necessarily defined in Control Panel), and removes them. It then loads the preferred keyboard layout defined below.

Currently the application only fixes the keyboard layout if it has focus.

Work in progress:
```C++
	// Fatal flaw of this application:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646305(v=vs.85).aspx
	// (LoadKeyboardLayout) This function has no effect if the current process does not own the window with keyboard focus.
	// @TODO workaround: use another process's foreground window to call GetKeyboardLayoutName() from user32.dll.
```