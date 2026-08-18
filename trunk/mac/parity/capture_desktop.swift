// Prints the CGWindowID of a desktop-parity-relevant window (current Space).
// Usage: swift capture_desktop.swift <finder|wallpaper|bumptop>
// Capture the window afterwards with: screencapture -x -l <id> out.png
import Cocoa
import CoreGraphics

let args = CommandLine.arguments
guard args.count == 2 else { fputs("usage: capture_desktop.swift <finder|wallpaper|bumptop>\n", stderr); exit(2) }
let mode = args[1]

let onScreen = CGWindowListCopyWindowInfo([.optionOnScreenOnly], kCGNullWindowID) as! [[String: Any]]
let desktopIconLevel = Int(CGWindowLevelForKey(.desktopIconWindow))

func findWindow(_ predicate: ([String: Any]) -> Bool) -> Int? {
    for w in onScreen where predicate(w) {
        return (w["kCGWindowNumber"] as! Int)
    }
    return nil
}

var result: Int? = nil
switch mode {
case "finder":
    result = findWindow { w in
        (w["kCGWindowOwnerName"] as? String) == "Finder" && (w["kCGWindowLayer"] as? Int) == desktopIconLevel
    }
case "wallpaper":
    result = findWindow { w in
        let name = (w["kCGWindowName"] as? String) ?? ""
        return (w["kCGWindowOwnerName"] as? String) == "Dock" && name.hasPrefix("Wallpaper")
    }
case "bumptop":
    result = findWindow { w in
        let b = w["kCGWindowBounds"] as? [String: Any] ?? [:]
        let width = (b["Width"] as? Double) ?? 0
        return (w["kCGWindowOwnerName"] as? String) == "BumpTop" && width > 100
    }
default:
    fputs("unknown mode\n", stderr); exit(2)
}

guard let id = result else { fputs("window not found: \(mode)\n", stderr); exit(1) }
print(id)
