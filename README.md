# `Haka`
Simply select text, press the key combination, and it's added to your file! Without cluttering your clipboard buffer with one-time-use text.

<p align="center">
  All using <b><i>Haka</i></b>, a low level global keyboard event listener for Wayland.
  <br/>
  <img src="https://github.com/user-attachments/assets/94bfdb3c-b3ee-4772-bb04-be72dfc07517" alt="Haka demo"/>
</p>

- [Installation](https://github.com/def3r/haka?tab=readme-ov-file#build)
  - [Permissions](https://github.com/def3r/haka?tab=readme-ov-file#permissions)
- [Daemon](https://github.com/def3r/haka?tab=readme-ov-file#hakaservice)
- [Configuration](https://github.com/def3r/haka?tab=readme-ov-file#config)
  - [Config Options](https://github.com/def3r/haka?tab=readme-ov-file#config-options)
- [Guide](https://github.com/def3r/haka?tab=readme-ov-file#guide)
- [Plugins](https://github.com/def3r/haka?tab=readme-ov-file#keybinds)
    - [Writing your own Plugin]()
- [Troubleshoot](https://github.com/def3r/haka?tab=readme-ov-file#troubleshoot)

---

## Project Aim
This project aims to solve a problem I face when making notes: efficiently
creating and organizing notes when you have a large volume of resources and
limited time.

### The Problem with Existing Solutions
<ol type="1">
  <li>
    <b><code>Highlighting</code></b> important points across different sources
(web pages, PDFs, ebooks) makes it hard to search for a <b>specific point</b>
in a centralized way.
  </li>
  <li>
    <code>Manual Copy Pasta</code> is extremely slow with many steps involved,
      <ol type="a">
        <li>Select the point <i>with your mouse</i></li>
        <li>Hit the <i><code>&lt;C-c&gt;</code></i></li>
        <li>Switch to the editor</li>
        <li>Hit the <i><code>&lt;C-v&gt;</code></i></li>
        <li>Switch back to the resource</li>
      </ol>
    This may not look a lot but this breaks the flow and I do get annoyed hitting so
many keystrokes.
  </li>
</ol>

### The Solution
What if life was as simple as:
<ol type="a">
  <li>Select the point <i>with your mouse (unfortunately)</i></li>
  <li>Hit a <i><code>&lt;Key-Combo&gt;</code></i></li>
</ol>
By eliminating the redundant steps, <i><code>Haka</code></i> allows you to capture key points
without breaking your concentration and also allows you to <i>fuzzy find</i> notes later.

## build
Dependencies
- *[`libevdev`](https://gitlab.freedesktop.org/libevdev/libevdev)*
- *[`wl-clipboard`](https://github.com/bugaevc/wl-clipboard)*
- *[`tofi`](https://github.com/philj56/tofi)*

*[`build.sh`](https://github.com/horrifyingHorse/haka/blob/main/build.sh)* includes the installation of all the dependencies, but only for arch and debian based distros. For any other distribution, kindly install the aforementioned dependencies. Or simply use the *based* [Makefile](https://github.com/horrifyingHorse/haka/blob/main/Makefile)

### For Arch / Debian based distros:
From your favourite terminal, execute the *build.sh* script for installation
```python
chmod +x build.sh
./build.sh
```

### Permissions
To access the *`input`* group and access the event devices using *libevdev*, one needs root privileges. Although running *`haka`* as the root user has many issues, the most critiacl being the current user session, including the *primary clipboard*, is not available when running as root.

To resolve this issue, we use the *[`capabilities(7)`](https://man7.org/linux/man-pages/man7/capabilities.7.html)* to grant *haka* the permission to change its *group ID* to *`input`*. Now *haka* can run as a user process and still be able to access the *evdev*. **Credit** for this workaround goes to ***[`this blog`](https://suricrasia.online/blog/turning-a-keyboard-into/#:~:text=Running%20external%20programs)*.**

> [!NOTE]
> *build.sh* handles this implicitly. If you used *build.sh* to build *haka*, this has been performed already.

```python
sudo setcap "cap_setgid=eip" ./haka.out
./haka.out     # no sudo required while running
```

## `haka.service`
If you want to use *haka* as a *[`new-style systemd`](https://www.freedesktop.org/software/systemd/man/latest/daemon.html#New-Style%20Daemons)* daemon to run in the background, use *[`daemon.sh`](https://github.com/horrifyingHorse/haka/blob/main/daemon.sh)* to generate the unit file for systemd.

*daemon.sh* creates the unit file required to run as a `--user` daemon and it creates a symlink of the unit file to `~/.config/systemd/user/`

> [!NOTE]
> Make sure to execute *daemon.sh* from the directory containing *`haka.out`*. If you renamed the executable, edit the `HAKA` variable in `daemon.sh`

```python
chmod +x daemon.sh
./daemon.sh
```

Check logs:
```python
journalctl --user -u haka.service -f
```

## Config
Config file for haka must be named *[`haka.cfg`](https://github.com/horrifyingHorse/haka/blob/main/haka.cfg)* which must be present in the same directory as the *haka executable*. The haka config parser allows you to use **command substitution** for configuration values. *For example, you can specify the editor dynamically using*:

```env
editor=$(which emacs)
```

### Config Options
| Option | Value | Description |
|--------|-------|-------------|
| editor | /path/to/editor/bin | Specify the editor to open files in |
| terminal | /path/to/terminal/bin | Specify the terminal to open editor in |
| notes-dir | /path/to/notes/dir | Custom path to notes dir |
| tofi-cfg | /path/to/tofi/cfg | Custom path for tofi.cfg file |
| plugins | /path/to/plugins/dir/ | Custom path for plugins dir |

## Guide
- The files displayed in the selection menu is the `notes/` directory and can be found in the directory containing *haka* executable
- The file is opened by default in *[`neovim`](https://github.com/neovim/neovim)*
- *`haka`* currently supports the first 249 `KEY_NAME` defined in *[`linux/input-event-codes.h`](https://raw.githubusercontent.com/whot/libevdev/refs/heads/master/include/linux/input-event-codes.h)*
- The defualt *`ActivationCombo`* is *`Ctr+Alt`*
- The default activation combo is set in the *[`src/binds.c:setActivationCombo_`](https://github.com/def3r/haka/blob/main/src/binds.c)*.

## Plugins
- The only way to add New custom keybinds is to make plugins. This might sound
  tedious but is simple.
- By default, haka comes with plugins. You can see them in *plugins/* directory.

A very basic example to extend a plugin (Assuming *haka* is already running). In *`plugins/default.c`*, add this function:
```c
static void hello_world(hakaCtx* ctx) {
  printf("Hello World!\n");
}
```

And under the `BEGIN_BIND ... END_BIND` section of the file, register your function by adding this
line:
```c
  Bind(hello_world, KEY_H);
```

Thats it! We have made a new keybind! now, compile the file, in *plugins/* directory run:
```sh 
make
```

Now press your *ActivationCombo*(by deafault its `RIGHT_ALT + RIGHT_CTRL`) + R
to reload the plugins and then press *ActivationCombo + H* and you can see
"Hello World!" printed on the terminal!

### Writing your own Plugin:
- A plugin is just a shared object (`.so`) inside the *`plugins/`* directory or
  the directory specified in the config file.
- A function **must** be of the prototype: `void func(hakaCtx *ctx)` to be `Bind`able. Refer *[`linux/input-event-codes.h`](https://raw.githubusercontent.com/whot/libevdev/refs/heads/master/include/linux/input-event-codes.h)* for `KEY_NAME` macros.

**Boilerplate for plugin:**
```c
#include "plug.h" // Can be found in include/

coreApi *api;

/*
  Your functions go here
*/

BEGIN_BIND
  /*
    Register your functions to keys here
  */
END_BIND
```
You can use the *api* ptr to call core feature functions and extract data from within the `ctx`.

**Compiling the plugin**
- Ensure that plugin is compiled using: `-fPIC -shared` flags

## Troubleshoot
- If some processes do not work/launch, this is usually because at the time of
  booting, required env variables were not set. This is a known issue.
  `systemctl --user restart haka` should be done after booting.

- If noo keybind is working, the issue is most probably with `keyd`. `keyd` and
  `haka` cannot run simultaneously. This is because `keyd` grabs the input
  devies, thus leaving `haka` to poll the input devices indefinitely. `kill`ing
  the `keyd` process should resolve the issue.<br>
  Confirm `keyd` is running: `pgrep keyd`.<br>
  To kill all procs with keyd: `killall -s 0 keyd`

## TODO
- [ ] Are `contextCheck` and `eventHandlerEpilogue` macros needed?
- [ ] Expose API for current key state(?)
- [ ] Warning for Multiple binds on a single key
- [ ] Static core.c
- [ ] Use execvp for path var finding
- [x] Log levels
- [ ] Switch to gtk(?): to reduce dependencies.
- [x] Add a config file option for vars.
- [x] Ignore newlines in selection(?)
- [x] Improve bindings implementation
