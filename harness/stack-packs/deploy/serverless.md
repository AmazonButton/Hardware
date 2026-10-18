# Stack Pack: Deploy — Serverless (Lambda/Cloud Functions/Workers)

> Nạp file này khi `harness.config.yaml → surfaces.infrastructure.stack: serverless-lambda | serverless-cloud-functions | cloudflare-workers`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Backend/API chạy dạng function-as-a-service, không quản lý server dài hạn.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Deploy (SAM/Serverless FW) | `sls deploy --stage <stage>  # hoặc: sam deploy` |
| Local invoke | `sls invoke local -f <function>` |
| Logs | `sls logs -f <function> -t` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Cold start: giữ dependency bundle nhỏ, tránh import cả SDK lớn khi chỉ cần 1 phần.
- Timeout function đặt tường minh, thấp hơn timeout của caller (API Gateway/Load Balancer) một khoảng an toàn.
- Idempotency key bắt buộc cho function xử lý side-effect (thanh toán, gửi email) — tránh double-execution khi retry.
- IAM role theo nguyên tắc least-privilege riêng cho từng function — không dùng 1 role chung cho toàn hệ thống.

## Lỗi thường gặp agent hay mắc (tránh)
- Kết nối DB mở mới mỗi lần invoke thay vì tái sử dụng connection pool ngoài handler (connection exhaustion).
- Không có DLQ (dead-letter queue) cho event/queue trigger → mất message khi lỗi liên tục.

## Quality gate bổ sung
Trace mọi lần deploy vào `harness-cli trace` (rollback plan bắt buộc cho lane high-risk).

## Tham chiếu
- https://www.serverless.com/framework/docs
