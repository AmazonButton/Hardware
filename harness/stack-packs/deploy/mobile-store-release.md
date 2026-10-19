# Stack Pack: Deploy — Mobile Store Release (Play Store/App Store)

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: play-store | app-store`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Release build mobile lên Google Play / Apple App Store.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Android bundle | `eas build --platform android --profile production  # hoặc: ./gradlew bundleRelease` |
| iOS build | `eas build --platform ios --profile production  # hoặc: xcodebuild archive` |
| Submit Android | `eas submit --platform android --latest` |
| Submit iOS | `eas submit --platform ios --latest` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Không submit trực tiếp lên production track — luôn qua internal/closed testing track trước.
- Release notes, screenshots, privacy policy URL cập nhật đồng bộ với behavior mới trước khi submit.
- Version code/build number tăng đơn điệu — không trùng/giảm giữa các lần submit.
- Kiểm tra chính sách Play/App Store mới nhất trước mỗi submit lớn (chính sách thay đổi thường xuyên).

## Lỗi thường gặp agent hay mắc (tránh)
- Quên cập nhật Data Safety form (Play) / App Privacy (App Store) khi thêm SDK/permission mới → reject.
- Rollout 100% ngay thay vì staged rollout (vd 10% → 50% → 100%) cho thay đổi rủi ro cao.

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md` + `release/PRE_RELEASE_CHECKLIST.md`. Bắt buộc rollback plan trước khi rollout > 50%.

## Tham chiếu
- failures/GPLAY-TEMPLATE.md
- release/RELEASE_PROCESS.md
