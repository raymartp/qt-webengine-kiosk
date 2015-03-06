qt-webengine-kiosk
===============

This is simple browser application written on Qt &amp; QtWebEngine, and is a dirty hackport of Sergey Dryabzhinsky's much more stable qt-webkit-kiosk. Because QtWebEngine gives much less control over the page than QtWebKit, it currently lacks some functionality that qt-webkit-kiosk has, such as UserJS, UserCSS, and even printing; it's unlikely that these can be reimplemented without updates to QtWebEngine itself. However, the updated web standards support of QtWebEngine mean that some (like myself) would benefit more from an up-to-date Blink engine than these features.

It requires Qt >= 5.4, as this is the first version to officially include qtwebengine.

Usualy runing in fullscreen mode, but supports maximized and fixed size window mode.

This browser uses only one application window. So, no popups, no plugins in separate processes, like Chrome do.

Supports several parameters via configuration file: proxy, user-agent, click sound.

Also hides printer dialog and uses default or defined printer.

##Downloads

TBA when I get some windows builds happening
