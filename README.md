# Tiny X Locker

TXLock is a simple, powerful and lightweight program for loccking the screen on UNIX-like systems with X11 (workability with Wayland is questionable).

Right now TXlock requires third-party programs (e.g. `xautolock`) to determine IDLE time, but this "problem" will be solved by integrating IDLE counter into program in the nearest future.

## Getting Started

### Installation

```bash
curl https://github.com/rendick/txlock/releases/latest/download/txlock --output txlock
sudo mv txlock /usr/local/bin/
```

### Solving the `root` problem

TXlock requires root privileges; without them the program will not work.

To fix that, you may lean towards this solution:

```bash
sudo chown root:root /usr/local/bin/txlock
sudo chmod 4755 /usr/local/bin/txlock
```

### Shortcuts

`Control + U` erases all entered text.

## License

[MIT](LICENSE)
