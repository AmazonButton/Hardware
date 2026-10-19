# Stack Pack: Mobile — Native Android (Kotlin/Compose)

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: native-android-compose`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
App Android thuần Kotlin/Jetpack Compose, không cross-platform.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `./gradlew test` |
| Lint | `./gradlew lint` |
| Build | `./gradlew assembleRelease` |
| Instrumented test | `./gradlew connectedAndroidTest` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Compose: state hoisting đúng cách — composable con nhận state qua tham số, không tự giữ state cần chia sẻ.
- Coroutines: scope theo lifecycle (`viewModelScope`/`lifecycleScope`) — không tự tạo `GlobalScope`.
- 16 KB page size alignment bắt buộc (Play Store 2026+) — kiểm tra native libs (.so) tương thích.
- `targetSdkVersion` theo yêu cầu Play Store hiện hành.

## Lỗi thường gặp agent hay mắc (tránh)
- Recomposition thừa do lambda mới tạo mỗi lần recompose — dùng `remember`.
- ANR do công việc nặng chạy trên Main dispatcher.

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md`.

## Tham chiếu
- https://developer.android.com/jetpack/compose
