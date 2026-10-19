#!/bin/bash
# Developer-only, optional: requires a Developer ID certificate in your Keychain
# and an existing notarytool keychain profile. Never put credentials in this repo.
set -euo pipefail
: "${LOWLINE_SIGN_IDENTITY:?Set LOWLINE_SIGN_IDENTITY to your Developer ID Application identity}"
: "${LOWLINE_NOTARY_PROFILE:?Set LOWLINE_NOTARY_PROFILE to your stored notarytool profile}"
package_dir="${1:?Usage: sign-mac.sh /path/to/package/Lowline}"
for bundle in "$package_dir/Standalone/Lowline.app" "$package_dir/AU/Lowline.component" "$package_dir/VST3/Lowline.vst3"; do
  codesign --force --options runtime --timestamp --sign "$LOWLINE_SIGN_IDENTITY" "$bundle"
  codesign --verify --strict --verbose=2 "$bundle"
done
archive="$(mktemp -d)/Lowline-notarisation.zip"
ditto -c -k --keepParent "$package_dir" "$archive"
xcrun notarytool submit "$archive" --keychain-profile "$LOWLINE_NOTARY_PROFILE" --wait
for bundle in "$package_dir/Standalone/Lowline.app" "$package_dir/AU/Lowline.component" "$package_dir/VST3/Lowline.vst3"; do
  xcrun stapler staple "$bundle"
  xcrun stapler validate "$bundle"
done
echo 'Signed and notarised bundles ready; create distribution archives from these bundles.'
