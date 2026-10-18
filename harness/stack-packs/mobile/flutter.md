# Stack Pack: Mobile — Flutter

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: flutter`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
App di động cross-platform bằng Flutter/Dart.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Analyze | `flutter analyze` |
| Test | `flutter test` |
| Build APK | `flutter build apk --release` |
| Build iOS | `flutter build ios --release` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Widget nặng tách riêng, dùng `const` constructor bất cứ khi nào có thể để giảm rebuild.
- State management nhất quán 1 pattern trong toàn app (Riverpod/Bloc) — không trộn lẫn tuỳ tiện.
- Localization qua `intl`/ARB files — không hardcode chuỗi.
- Null-safety nghiêm ngặt — không dùng `!` (bang operator) khi chưa chắc chắn giá trị non-null.

## Lỗi thường gặp agent hay mắc (tránh)
- `setState` gọi trong widget con thay vì nâng state lên đúng cấp → rebuild toàn cây thừa.
- Không test trên thiết bị thật cho performance (simulator/emulator không phản ánh đúng jank).

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md`.

## Tham chiếu
- https://docs.flutter.dev
