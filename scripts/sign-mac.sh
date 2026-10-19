#!/bin/bash
# Signs (Developer ID, hardened runtime), notarises and staples the three Mac bundles.
# Used by CI when the Apple secrets exist; can also be run locally with a Developer ID
# certificate in your Keychain and a stored notarytool profile. Never put credentials in this repo.
#   LOWLINE_SIGN_IDENTITY   "Developer ID Application: Name (TEAMID)"
#   LOWLINE_NOTARY_PROFILE  name given to `xcrun notarytool store-credentials`
#   LOWLINE_KEYCHAIN        optional keychain holding identity and profile (CI uses a temporary one)
set -euo pipefail
: "${LOWLINE_SIGN_IDENTITY:?Set LOWLINE_SIGN_IDENTITY to your Developer ID Application identity}"
: "${LOWLINE_NOTARY_PROFILE:?Set LOWLINE_NOTARY_PROFILE to your stored notarytool profile}"
package_dir="${1:?Usage: sign-mac.sh /path/to/package/Lowline}"
here="$(cd "$(dirname "$0")" && pwd)"
keychain_args=()
if [[ -n "${LOWLINE_KEYCHAIN:-}" ]]; then keychain_args=(--keychain "$LOWLINE_KEYCHAIN"); fi
bundles=("$package_dir/Standalone/Lowline.app" "$package_dir/AU/Lowline.component" "$package_dir/VST3/Lowline.vst3")
for bundle in "${bundles[@]}"; do
  entitlements=()
  # The standalone app records the bass input: the hardened runtime needs the audio-input entitlement.
  if [[ "$bundle" == *.app ]]; then entitlements=(--entitlements "$here/../packaging/Lowline.entitlements"); fi
  codesign --force --options runtime --timestamp ${entitlements[@]+"${entitlements[@]}"} \
    ${keychain_args[@]+"${keychain_args[@]}"} --sign "$LOWLINE_SIGN_IDENTITY" "$bundle"
  codesign --verify --strict --verbose=2 "$bundle"
done
archive="$(mktemp -d)/Lowline-notarisation.zip"
ditto -c -k --keepParent "$package_dir" "$archive"
xcrun notarytool submit "$archive" --keychain-profile "$LOWLINE_NOTARY_PROFILE" ${keychain_args[@]+"${keychain_args[@]}"} --wait
for bundle in "${bundles[@]}"; do
  xcrun stapler staple "$bundle"
  xcrun stapler validate "$bundle"
done
spctl --assess --type execute -vv "$package_dir/Standalone/Lowline.app"
echo 'Signed and notarised bundles ready; create distribution archives from these bundles.'
