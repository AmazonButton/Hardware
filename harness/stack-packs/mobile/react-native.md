# Stack Pack: Mobile — React Native / Expo

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: expo-react-native`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
App di động cross-platform bằng Expo + React Native, TypeScript.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Validate | `npm run validate` |
| Test | `npm run test -- --run` |
| EAS preview build | `eas build --platform android --profile preview` |
| EAS production build | `eas build --platform android --profile production` |
| Submit | `eas submit --platform android --latest` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Không hardcode chiều cao pixel — dùng flexbox/`Dimensions`/`useWindowDimensions`.
- Mọi text hiển thị người dùng qua `useTranslation()` — không hardcode chuỗi.
- Ảnh: SVG hoặc WebP, không PNG > 100KB.
- Safe area insets bắt buộc trên mọi screen (`react-native-safe-area-context`).
- 64-bit ABI only: `arm64-v8a` + `x86_64`. `minSdkVersion=24`, `targetSdkVersion` theo yêu cầu Play Store hiện hành (kiểm tra mỗi năm).
- 16 KB page alignment: `android.use16kPageSize=true` (bắt buộc từ 2026 cho Play Store).

## Lỗi thường gặp agent hay mắc (tránh)
- Quên test trên máy Android bản thấp RAM → OOM crash không thấy trên simulator mạnh.
- Deep link chưa test cold-start (app chưa mở) vs warm-start.
- Push notification permission flow không đúng thứ tự trên iOS 18+/Android 14+.

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md`. Bắt buộc chạy pipeline theo `MOBILE RELEASE PIPELINE` trong `agent-instructions/HIERARCHICAL_AGENTS.md`.

## Tham chiếu
- https://docs.expo.dev
- skills/android-debug/SKILL.md
