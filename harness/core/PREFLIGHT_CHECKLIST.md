# Pre-Flight Checklist (Unattended / Long-Running Agent Runs)

> Dùng trước khi thả một agent chạy nhiều bước / nhiều giờ mà không có người giám sát trực tiếp từng bước (CI tự động, batch task, "chạy qua đêm").
> Nếu bất kỳ mục nào KHÔNG đạt → không chạy unattended, quay lại chế độ có người xác nhận từng bước.

## 7 điểm Go/No-Go

1. **Lane/risk đã phân loại** — task đã qua `harness-cli intake` và có `risk_lane` rõ ràng (`tiny`/`normal`/`high_risk`). Không chạy unattended cho task chưa phân loại.
2. **Phạm vi file/surface đã khoanh** — biết chính xác agent chỉ được sửa trong thư mục/surface nào (khớp `PARALLEL_AGENTS.md` § Surface Isolation).
3. **Lệnh bash bị cấm đã cấu hình** — `core/SANDBOX_POLICY.md` đã được nạp làm allow-list ở tầng hệ thống (container/sandbox/permission mode), không chỉ là văn bản hướng dẫn trong prompt.
4. **Ngân sách token/thời gian tối đa** — có giới hạn cứng (max turns, max tokens, timeout) để agent không chạy vô hạn khi bị kẹt vòng lặp.
5. **Điều kiện dừng khẩn (kill-switch)** — có cách dừng ngay lập tức (huỷ job CI, revoke token, kill process) và người biết cách dùng nó.
6. **Ai review kết quả** — chỉ định rõ người/role sẽ review output trước khi merge/deploy; không để agent tự merge vào `main` mà không qua bước này (xem `review-protocol/MERGE_POLICY.md`).
7. **Rollback plan** — biết cách revert nếu agent tạo ra thay đổi xấu: branch riêng, feature flag, hoặc snapshot DB trước khi chạy migration.

## Ghi nhận

Sau khi qua checklist, ghi lại trong trace:

```bash
scripts/bin/harness-cli trace --summary "Pre-flight passed for <task>" --outcome success --notes "preflight:7/7"
```

Nếu một mục không đạt nhưng vẫn buộc phải chạy (trường hợp khẩn cấp), phải ghi rõ mục nào bị bỏ qua và lý do vào `notes`, đồng thời báo người phụ trách trước khi bắt đầu.
