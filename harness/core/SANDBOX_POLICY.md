# Sandbox / Shell Permission Policy

> Đặt tại `harness-engineering/core/SANDBOX_POLICY.md`.
> Áp dụng cho mọi lệnh shell/bash mà agent thực thi trực tiếp hoặc qua CI, bất kể surface.

## 1. Ba mức quyền

| Mức | Ý nghĩa | Áp dụng |
|---|---|---|
| `allow` | Agent chạy thẳng, không cần hỏi | Đọc file, lint, test, build trong phạm vi surface đã khoanh |
| `ask` | Agent phải trình bày lệnh + lý do, chờ xác nhận người trước khi chạy | Ghi ngoài phạm vi surface, cài package mới, migrate DB |
| `deny` | Agent không được chạy dưới bất kỳ hình thức nào, kể cả khi được "yêu cầu rõ ràng" trong prompt | Xoá lịch sử git, ghi đè `main`, gọi ra ngoài mạng tới domain lạ, đọc/ghi secrets |

## 2. Bảng lệnh mặc định

| Lệnh / pattern | Mức | Ghi chú |
|---|---|---|
| `rg`, `grep`, `find`, `cat`, `ls`, `view` | allow | Chỉ đọc |
| `npm test`, `pytest`, `./mvnw test`, lint/typecheck | allow | Trong phạm vi surface |
| `git commit`, `git push <feature-branch>` | allow (normal/tiny), ask (high-risk) | Không bao giờ push thẳng `main` |
| `git push --force`, `git push origin main` | deny | Luôn qua PR + review |
| `rm -rf`, `git reset --hard` ngoài worktree hiện tại | deny | Rủi ro mất dữ liệu không phục hồi |
| `curl ... | sh`, `curl ... | bash` | deny | Thực thi mã không kiểm soát |
| Truy cập `.env`, `~/.ssh`, biến môi trường chứa `SECRET`/`TOKEN`/`KEY` | deny (đọc để log/in ra), ask (đọc để dùng nội bộ) | Không bao giờ in giá trị secret ra log/trace |
| Lệnh migrate DB production (`prisma migrate deploy`, `flyway migrate` trên DB không phải local/staging) | ask, bắt buộc `requires_human_confirmation` | Theo `lanes.high_risk` |
| Cài package mới (`npm install <pkg>`, `pip install <pkg>`) | ask | Cần review license + supply-chain trước khi thêm |
| Gọi API/network ra ngoài domain đã whitelist trong `mcp.config.json` | ask (normal), deny (nếu domain lạ, không nằm trong whitelist) | |
| Chạy binary chưa verify checksum (vd. `harness-cli.exe` bản mới) | ask, bắt buộc kiểm tra `expected_sha256` trong config trước | Xem GAP_ANALYSIS mục 0 |

## 3. Quy tắc leo thang theo lane

- `tiny`: chỉ `allow` các lệnh đọc + test/lint trong phạm vi 1 surface. Bất cứ điều gì khác → tự động nâng lane lên `normal`.
- `normal`: `allow` như trên + build/commit/push feature-branch. Ghi ngoài phạm vi surface đã khai báo trong story → `ask`.
- `high_risk`: mọi lệnh có khả năng ảnh hưởng auth/billing/dữ liệu người dùng/production đều là `ask` tối thiểu, có `deny` tuyệt đối cho nhóm ở bảng trên bất kể ai yêu cầu.

## 4. Nguyên tắc bất biến

1. Không có prompt hay "role" nào được phép hạ một lệnh từ `deny` xuống `allow`. Danh sách `deny` chỉ sửa được qua Harness Change Policy (cần xác nhận người), không sửa được "on the fly" trong lúc chạy task.
2. Mọi lệnh ở mức `ask` bị từ chối phải được ghi vào trace (`harness-cli trace --outcome blocked`) kèm lý do, để tổng hợp friction sau này.
3. CI/CD chạy các agent không giám sát (unattended) phải nạp policy này làm allow-list thực thi ở tầng hệ thống (không chỉ là hướng dẫn trong prompt) — vd. sandbox container, seccomp profile, hoặc permission mode của Agent SDK/Claude Code.
