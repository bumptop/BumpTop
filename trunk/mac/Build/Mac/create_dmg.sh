source="Deploy"
title="BumpTop"
finalDMGName="Deploy/BumpTop.dmg"
size="100000"
backgroundPictureName="dmg-background.png"
applicationName="BumpTop"

echo hdiutil detach ${device}
hdiutil detach ${device}
rm "pack.temp.dmg"
echo rm ${finalDMGName}
rm ${finalDMGName}

echo  hdiutil create -srcfolder "${source}" -volname "${title}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${size}k pack.temp.dmg
hdiutil create -srcfolder "${source}" -volname "${title}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${size}k pack.temp.dmg
device=$(hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $1}')
echo $device

echo mkdir /Volumes/${title}/.background
mkdir /Volumes/${title}/.background
echo cp ${backgroundPictureName} /Volumes/${title}/.background
cp ${backgroundPictureName} /Volumes/${title}/.background

echo '
   tell application "Finder"
     tell disk "'${title}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 992, 445}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 120
           set background picture of theViewOptions to file ".background:'${backgroundPictureName}'"
           make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
           set position of item "'${applicationName}'" of container window to {140, 165}
           set position of item "Applications" of container window to {453, 165}
           close
           open
           update without registering applications
           delay 5
           eject
     end tell
   end tell
' | osascript

chmod -Rf go-w /Volumes/"${title}"
sync
sync
hdiutil detach ${device}
hdiutil convert "pack.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
rm -f pack.temp.dmg
