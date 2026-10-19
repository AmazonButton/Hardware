# Golden Path: Data Pipeline / Background Worker Archetype

## 1. Overview
Dành cho batch job, ETL, consumer hàng đợi (queue consumer), scheduled worker — không có giao diện người dùng, thường chạy nền hoặc theo lịch/CI.

---

## 2. Recommended Tech Stack
- **Option A (JVM ecosystem)**: Java/Kotlin + Spring Batch hoặc Kafka Streams
- **Option B (Python/Data)**: Python + Airflow/Dagster/Prefect cho orchestration, Pandas/Polars cho xử lý
- **Option C (Go/Node)**: Go worker pool hoặc Node.js + BullMQ cho queue consumer nhẹ
- **Queue/Broker**: Kafka, RabbitMQ, SQS, hoặc Redis Streams
- **Observability**: structured logging (JSON) + metrics (Prometheus) + dead-letter queue giám sát riêng

---

## 3. Directory Layout (tham khảo)

```
├── src/
│   ├── jobs/              # 1 file / 1 job hoặc 1 pipeline step
│   ├── connectors/         # nguồn/đích dữ liệu (DB, API, file, queue)
│   ├── transforms/         # logic biến đổi dữ liệu, thuần túy, dễ unit test
│   └── scheduling/         # cron/DAG definition
├── tests/
│   ├── unit/               # test transform logic độc lập với I/O
│   └── integration/        # test với DB/queue test container
├── ops/
│   ├── dlq-runbook.md      # cách xử lý dead-letter queue
│   └── backfill.md         # quy trình chạy lại dữ liệu quá khứ an toàn
└── AGENTS.md
```

---

## 4. Quality Gates Checklist

| Stage | Command | Threshold |
| :--- | :--- | :--- |
| Unit tests (transform logic) | test runner tương ứng stack | ≥ 80% coverage cho `transforms/` |
| Integration test (DB/queue thật qua testcontainer) | | Không dùng mock cho phần ghi dữ liệu cuối (sink) |
| Idempotency check | chạy lại cùng 1 batch 2 lần | Kết quả cuối giống hệt (không tạo bản ghi trùng) |
| Backfill dry-run | chạy ở chế độ dry-run trước khi backfill thật | Bắt buộc cho mọi backfill trên dữ liệu production |
| Dead-letter monitoring | alert khi DLQ tăng bất thường | Ngưỡng cấu hình trong `quality_gates` |
| Data contract | schema đầu ra khớp `docs/product/*` hoặc schema registry | 0 breaking change không qua decision record |

## 5. Lưu ý đặc thù Pipeline
- Đây là lane mặc định `normal` trở lên cho mọi job đụng tới dữ liệu production; backfill/migrate dữ liệu lớn mặc định `high_risk` (theo `core/SANDBOX_POLICY.md` — lệnh ghi dữ liệu hàng loạt cần `ask`/xác nhận người).
- Ưu tiên thiết kế idempotent (chạy lại an toàn) hơn là dựa vào "chỉ chạy đúng 1 lần" — agent và cả CI đều có thể vô tình chạy lại 1 job.
