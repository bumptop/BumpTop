// Captures the main display showing ONLY the given windows (by CGWindowID),
// via ScreenCaptureKit. Used by the parity harness to grab the wallpaper +
// Finder icon layer (or the BumpTop window) without other windows on top.
// Usage: capture_window <out.png> <windowID> [windowID...]
import Cocoa
import ScreenCaptureKit

_ = NSApplication.shared  // establish a WindowServer connection for SkyLight

let args = CommandLine.arguments
guard args.count >= 3 else {
    fputs("usage: capture_window <out.png> <windowID> [windowID...]\n", stderr); exit(2)
}
let outPath = args[1]
let windowIDs = Set(args[2...].compactMap { UInt32($0) })

let semaphore = DispatchSemaphore(value: 0)
var exitCode: Int32 = 1

Task {
    do {
        let content = try await SCShareableContent.excludingDesktopWindows(false, onScreenWindowsOnly: false)
        guard let display = content.displays.first else {
            fputs("no display\n", stderr); semaphore.signal(); return
        }
        let windows = content.windows.filter { windowIDs.contains($0.windowID) }
        guard !windows.isEmpty else {
            fputs("no matching windows in shareable content\n", stderr); semaphore.signal(); return
        }
        let filter = SCContentFilter(display: display, including: windows)
        let config = SCStreamConfiguration()
        let scale = 2
        config.width = Int(display.width) * scale
        config.height = Int(display.height) * scale
        config.showsCursor = false
        config.captureResolution = .best
        let image = try await SCScreenshotManager.captureImage(contentFilter: filter, configuration: config)
        let rep = NSBitmapImageRep(cgImage: image)
        guard let png = rep.representation(using: .png, properties: [:]) else {
            fputs("png encode failed\n", stderr); semaphore.signal(); return
        }
        try png.write(to: URL(fileURLWithPath: outPath))
        print("wrote \(outPath) \(image.width)x\(image.height)")
        exitCode = 0
    } catch {
        fputs("capture failed: \(error)\n", stderr)
    }
    semaphore.signal()
}
semaphore.wait()
exit(exitCode)
