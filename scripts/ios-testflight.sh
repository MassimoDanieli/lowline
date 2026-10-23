#!/bin/bash
# Radice for iPad / iPhone -> TestFlight, from your Mac, with Xcode's automatic signing.
#
# Once:  1. Xcode (16 or newer) > Settings > Accounts: sign in with your Apple Developer account.
#        2. App Store Connect > Apps > + New App: iOS, name Radice, bundle ID com.massimodanieli.lowline
#           (the bundle ID keeps its original value; only the visible name changed).
#           (register it under Certificates, Identifiers & Profiles if it is not offered).
# Then:  LOWLINE_TEAM_ID=XXXXXXXXXX scripts/ios-testflight.sh
#        The build shows up in App Store Connect > TestFlight after Apple's processing (~10-30 min).
set -euo pipefail
team="${LOWLINE_TEAM_ID:?Set LOWLINE_TEAM_ID to your Team ID (developer.apple.com/account > Membership details)}"
cd "$(dirname "$0")/.."
# CFBundleVersion must grow with every upload: date and time, e.g. 251019.1730.
build_number="${LOWLINE_BUILD_NUMBER:-$(date +%y%m%d.%H%M)}"
out=build-ios
cmake -S . -B "$out" -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
  -DCMAKE_XCODE_GENERATE_SCHEME=ON -DLOWLINE_TEAM_ID="$team" -DLOWLINE_BUILD_NUMBER="$build_number"
rm -rf "$out/Lowline.xcarchive" "$out/export"
xcodebuild -project "$out/Lowline.xcodeproj" -scheme Lowline_Standalone -configuration Release \
  -destination 'generic/platform=iOS' -archivePath "$out/Lowline.xcarchive" -allowProvisioningUpdates archive
cat > "$out/ExportOptions.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>method</key><string>app-store-connect</string>
  <key>destination</key><string>upload</string>
  <key>teamID</key><string>$team</string>
  <key>signingStyle</key><string>automatic</string>
  <key>uploadSymbols</key><true/>
  <key>manageAppVersionAndBuildNumber</key><false/>
</dict>
</plist>
PLIST
xcodebuild -exportArchive -archivePath "$out/Lowline.xcarchive" -exportOptionsPlist "$out/ExportOptions.plist" \
  -exportPath "$out/export" -allowProvisioningUpdates
echo "Uploaded Radice build $build_number. Add testers in App Store Connect > TestFlight."
