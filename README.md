# PluginHotInput
Rainmeter plugin for raw input support.


https://user-images.githubusercontent.com/77834863/153483347-765a09cc-b26f-49ca-8cbb-d18b4064f8da.mp4


# Disclaimer:
This is a plugin meant for advanced users. You'd most probably need a script, [PowershellRM](https://github.com/khanhas/PowershellRM) or [Lua](https://docs.rainmeter.net/manual/lua-scripting/) to work with it. The example uses a Lua script to create a minimal text field. But you are limited to your imagination.

For any suggestions, changes, or additions, open an [issue](https://github.com/deathcrafter/PluginHotInput/issues).

# Usage:
## Options:
- **InputAction**:
  - Action to be executed on input. `$input$` is replaced by [input strings](#input-strings).
- **NavigateAction**:
  - Action to be executed on navigation, like UP, DOWN, LEFT, RIGHT etc. `$navigation$` is replaced by [navigation strings](#navigation-strings).
- **UseLuaEscape**:
  - Escapes lua special characters, in case you are using lua functions with CommandMeasure.

## Input Strings:
- `LINEFEED`
- `SPACE`
- `TAB`
- `BACKSPACE`
- `DELETE`
- `ESC`
- `RETURN`

- Literal characters, e.g. `a`, `ま`, `ë` etc

### Extras:
- `COPY`
- `CUT`
- `PASTE`
- `SAVE`
- `UNDO`
- `REDO`

## Navigation Strings:
- `DOWN`
- `LEFT`
- `RIGHT`
- `UP`

- `END`
- `HOME`

### Extras:
- `CTRLDOWN`
- `SHIFTDOWN`
- `CTRLLEFT`
- `SHIFTLEFT`
- `CTRLRIGHT`
- `SHIFTRIGHT`
- `CTRLUP`
- `SHIFTUP`
- `SELECTALL`
