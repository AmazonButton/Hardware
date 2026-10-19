# Token & Cost Tracking

## 1. Mục tiêu
`trace.token_estimate` đã tồn tại trong schema v1 nhưng không đủ để tối ưu: không biết role nào tốn token nhất, loại task nào đắt nhất. Schema `003-cost-tracking.sql` (xem `scripts/schema/`) thêm bảng `trace_role_cost` để ghi chi tiết theo role, và view `v_cost_by_role`.

## 2. Cách ghi nhận
Khi mỗi role trong Fan-out/Fan-in kết thúc review, ghi 1 dòng:

```bash
scripts/bin/harness-cli trace-cost add \
  --trace-id <id> \
  --role security_auditor \
  --tokens-used 4200 \
  --duration-seconds 38
```

(Nếu CLI bản hiện tại chưa hỗ trợ lệnh `trace-cost`, ghi tạm vào `trace.notes` dạng JSON cho tới khi CLI được cập nhật — xem backlog.)

## 3. Dùng dữ liệu để tối ưu
- Chạy `harness-cli query cost --by role` định kỳ (hàng tuần) để xem role nào tốn token nhất so với tỷ lệ tìm ra lỗi thật (so với `role_review_report` PASS/BLOCK).
- Nếu 1 role gần như luôn PASS mà vẫn tốn token đều đặn cho lane `tiny`/`normal` ít rủi ro → cân nhắc tắt role đó cho các lane thấp rủi ro trong `harness.config.yaml → roles`.
- Đưa số liệu này vào `metrics/BENCHMARK_PROTOCOL.md` như một chỉ số harness maturity (H4 trở lên: tối ưu hoá dựa trên dữ liệu đo được, không phải cảm tính).

## 4. Ngân sách theo lane (khuyến nghị khởi điểm)

| Lane | Ngân sách token / task (khuyến nghị) | Số role tối đa kích hoạt |
|---|---|---|
| tiny | ≤ 8,000 | 1 (orchestrator tự xử lý, bỏ qua role cycle) |
| normal | ≤ 30,000 | tối đa 4 role liên quan trực tiếp surface bị đổi |
| high_risk | ≤ 80,000 | đủ 7 role, chấp nhận chi phí cao hơn để đổi lấy an toàn |

Ngân sách chỉ là điểm khởi đầu — điều chỉnh dựa trên dữ liệu `v_cost_by_role` thực tế của dự án, không copy nguyên số này.
