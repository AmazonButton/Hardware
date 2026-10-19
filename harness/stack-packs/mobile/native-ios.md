# Stack Pack: Mobile — Native iOS (Swift/SwiftUI)

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: native-ios-swiftui`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
App iOS thuần Swift/SwiftUI, không cross-platform.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `xcodebuild test -scheme <Scheme> -destination 'platform=iOS Simulator,name=iPhone 16'` |
| Build | `xcodebuild -scheme <Scheme> -configuration Release` |
| Lint | `swiftlint` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- SwiftUI: state qua `@State`/`@Observable` (Observation framework) — tránh Combine cho state đơn giản mới.
- Concurrency: dùng `async/await` + `actor` cho shared mutable state — tránh callback lồng nhau mới.
- Không force-unwrap (`!`) optional trừ khi có comment giải thích invariant.
- Tuân thủ Human Interface Guidelines cho mọi control chuẩn (không tự vẽ lại control hệ thống trừ khi có lý do UX rõ ràng).

## Lỗi thường gặp agent hay mắc (tránh)
- Retain cycle do closure giữ `self` mạnh trong callback dài hạn — dùng `[weak self]`.
- Quên test trên chế độ Dynamic Type lớn / VoiceOver (a11y gate).

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md`.

## Tham chiếu
- https://developer.apple.com/documentation/swiftui
