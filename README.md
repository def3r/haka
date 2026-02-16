# `Haka`
> [!IMPORTANT]
> *Haka has become more than just a [notes utility](./NEED.md) it used to be.
> Those features are now shipped as [`plugins`](#plugins) by default.*

*Haka* is a lightweight, fast, `pluggable` automation tool. It is designed to
run quietly in the background as a `system-service`.

Written completely in C for performance, but it <ins>*does not*</ins>* require
you to write your plugins *entirely* in C. It is dynamic and `hot-reloadable`!

<!-- <p align="center"> -->
<!--   <br/> -->
<!--   <img src="https://github.com/user-attachments/assets/94bfdb3c-b3ee-4772-bb04-be72dfc07517" alt="Haka demo"/> -->
<!-- </p> -->

# Index
- [Installation](#build)
  - [Permissions](#permissions)
- [Daemon](#hakaservice)
- [Configuration](#config)
  - [Config Options](#config-options)
- [Guide](#guide)
  - [Default KeyBinds](#default-keybinds)
- [Plugins](#plugins)
  - [Writing your own Plugin](#writing-your-own-plugin)
- [Troubleshoot](#troubleshoot)

## build
Dependencies
- *[`libevdev`](https://gitlab.freedesktop.org/libevdev/libevdev)*
- *[`wl-clipboard`](https://github.com/bugaevc/wl-clipboard)*
- *[`tofi`](https://github.com/philj56/tofi)*

*[`build.sh`](./build.sh)*
includes the installation of all the dependencies, but only for arch and debian
based distros. For any other distribution, kindly install the aforementioned
dependencies. Or simply use the *based*
[Makefile](.//Makefile)

### For Arch / Debian based distros:
From your favourite terminal, execute the *build.sh* script for installation
```python
chmod +x build.sh
./build.sh
```

### Permissions
To access the *`input`* group and access the event devices using *libevdev*,
one needs root privileges. Although running *`haka`* as the root user has many
issues, the most critiacl being the current user session, including the
*primary clipboard*, is not available when running as root.

To resolve this issue, we use the
*[`capabilities(7)`](https://man7.org/linux/man-pages/man7/capabilities.7.html)*
to grant *haka* the permission to change its *group ID* to *`input`*. Now
*haka* can run as a user process and still be able to access the *evdev*.
**Credit** for this workaround goes to ***[`this
blog`](https://suricrasia.online/blog/turning-a-keyboard-into/#:~:text=Running%20external%20programs)*.**

> [!NOTE]
> *build.sh* handles this implicitly. If you used *build.sh* to build
> *haka*, this has been performed already.

```python
sudo setcap "cap_setgid=eip" ./haka.out
./haka.out     # no sudo required while running
```

## `haka.service`
If you want to use *haka* as a *[`new-style
systemd`](https://www.freedesktop.org/software/systemd/man/latest/daemon.html#New-Style%20Daemons)*
daemon to run in the background, use
*[`daemon.sh`](https://github.com/horrifyingHorse/haka/blob/main/daemon.sh)* to
generate the unit file for systemd.

*daemon.sh* creates the unit file required to run as a `--user` daemon and it
creates a symlink of the unit file to `~/.config/systemd/user/`

> [!NOTE]
> Make sure to execute *daemon.sh* from the directory containing
> *`haka.out`*. If you renamed the executable, edit the `HAKA` variable in
> `daemon.sh`

```python
chmod +x daemon.sh
./daemon.sh
```

Check logs:
```python
journalctl --user -u haka.service -f
```

## Config
Config file for haka must be named
*[`haka.cfg`](https://github.com/horrifyingHorse/haka/blob/main/haka.cfg)*
which must be present in the same directory as the *haka executable*. The haka
config parser allows you to use **command substitution** for configuration
values. *For example, you can specify the editor dynamically using*:

```env
editor=$(which emacs)
```

### Config Options
The below config options are described with respect to the [`markdown` plugin](plugins/markdown/).

| Option | Value | Description |
|--------|-------|-------------|
| editor | /path/to/editor/bin | Specify the editor to open files in |
| terminal | /path/to/terminal/bin | Specify the terminal to open editor in |
| notes-dir | /path/to/notes/dir | Custom path to notes dir |
| tofi-cfg | /path/to/tofi/cfg | Custom path for tofi.cfg file |
| plugins | /path/to/plugins/dir/ | Custom path for plugins dir |

## Guide
- Haka comes with a `deafult` plugin and `markdown` plugin (*haka* exclusively
  used to be a notes utility).
- The files displayed in the selection menu is the `notes/` directory and can
  be found in the directory containing *haka* executable
- The `editor` by default in *[`neovim`](https://github.com/neovim/neovim)*
- *`haka`* currently supports the first 249 `KEY_NAME` defined in
  *[`linux/input-event-codes.h`](https://raw.githubusercontent.com/whot/libevdev/refs/heads/master/include/linux/input-event-codes.h)*
- If a key is binded to `NULL`, that key would be implicitly binded to *reload
  plugins*
- The defualt *`ActivationCombo`* is *`Left Ctr+Left Alt`*
- The default activation combo is set in the
  *[`src/binds.c:setActivationCombo_`](src/binds.c)*.
- In case of a `KEY_` binded to multiple functions, the first plugin to bind it
  would be executed.
- There are 3 *log levels*:
  - `-DLOG=0`: No logs
  - `-DLOG=1`: Info logs (`ILOG`)
  - `-DLOG=2`: Debug logs (`DLOG` + `ILOG`) *default*

### Default KeyBinds
- Default `ActivationCombo` is `LEFT_CTRL + LEFT_ALT`

<details>

<summary>plugins/default.c</summary>

| Key | Action |
|-----|--------|
| *`KEY_R`* | Reload plugins |
| *`KEY_C`* | Append selected text to current file |
| *`KEY_M`* | Display `tofi` selection menu |
| *`KEY_O`* | Open current file in `editor` |
| *`KEY_F`* | Format(wrap) current file content (using `neovim`) |
| *`KEY_J`* | Open `journalctl` of haka in `terminal` (if running as daemon) |

</details>

<details>

<summary>plugins/markdown/plugin.c</summary>

| Key | Action |
|-----|--------|
| *`KEY_1`* to *`KEY_4`* | Append selected text as `Hn` heading to current file |
| *`KEY_A`* | Append selected text as `unordered list` to current file |
| *`KEY_S`* | Append selected text as `unordered list` depth 2 to current file |
| *`KEY_N`* | Append newline(`\n`) to current file |

</details>

## Plugins
- The only way to add New custom keybinds is to make plugins. This might sound
  tedious but is simple.
- By default, haka comes with some plugins. You can see them in *plugins/* directory.

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

Now press your *ActivationCombo*(by deafault its `LEFT_ALT + LEFT_CTRL`) + R
to reload the plugins and then press *ActivationCombo + H* and you can see
"Hello World!" printed on the terminal!

### Writing your own Plugin
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
  To kill all procs with keyd: `sudo killall keyd`

## TODO
- [ ] [This architecture](https://www.reddit.com/r/linuxquestions/comments/1cn3ylp/how_to_make_sure_that_a_daemon_or_systemd_service/) makes sense
- [ ] Instead use `prefix key`(?) (multiple benefits of switching tbh)
- [ ] Warning for Multiple binds on a single key (Or maybe allow multiple binds?)
- [x] Static core.c
- [x] Log levels
- [ ] Switch to gtk(?): to reduce dependencies. (bloat tho)
- [x] Add a config file option for vars.
- [x] Ignore newlines in selection(?)
- [x] Improve bindings implementation

---

<sub>This project was inspired [out of personal disgust to `<C-c> <C-v>`](./NEED.md)</sub>
