# Solutioning Gate Report: <Tên initiative>

> Viết bởi role `architect` (không phải người viết PRD — giữ nguyên tắc Fresh-Context
> Evaluator, xem `roles/ROLE_SYSTEM.md`). Default-FAIL: mọi tiêu chí PASS phải có
> `evidence_ref` trỏ tới phần cụ thể trong PRD/Architecture/UX Spec.

## Liên kết
- Gate id (harness-cli): <id>
- Brief / PRD / Architecture / UX Spec: <đường dẫn>

## Checklist

| Tiêu chí | Verdict | evidence_ref |
|---|---|---|
| PRD trả lời rõ vấn đề + mục tiêu đo lường được | | |
| Mọi câu hỏi mở trong PRD đã được Architecture trả lời | | |
| Tech stack đã chọn có stack-pack tương ứng (hoặc đã thêm mới) | | |
| Không mâu thuẫn với CONSTITUTION.md | | |
| Không mâu thuẫn với compliance/domain-packs (nếu có domain đặc thù) | | |
| Rủi ro kỹ thuật lớn đã có phương án giảm thiểu | | |
| (Nếu có UI) UX Spec khả thi với stack đã chọn | | |
| Story packet đề xuất đủ nhỏ để fit vào risk lane tương ứng | | |

## Verdict tổng
`PASS` / `CONCERNS` / `FAIL`

## Nếu CONCERNS hoặc FAIL — hành động tiếp theo bắt buộc
<Không được tạo story packet cho tới khi giải quyết xong>

## Ghi lại kết quả
```bash
./scripts/bin/harness-cli gate decide <id> --verdict <pass|concerns|fail> \
  --reviewed-by architect --notes "..." --story-ids '["US-...","US-..."]'
```
