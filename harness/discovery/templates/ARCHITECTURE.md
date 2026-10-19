# Architecture: <Tên initiative>

> Viết bởi role `architect`, dựa trên PRD đã duyệt. Đây là artifact quyết định Solutioning
> Gate PASS/CONCERNS/FAIL nhiều nhất — phải trả lời được mọi câu hỏi mở trong PRD.

## Liên kết
- PRD: discovery/prds/<slug>.md
- UX Spec (nếu có): discovery/ux-specs/<slug>.md

## Tech stack theo surface (nối với stack-packs/)
| Surface | Stack | Stack-pack tham chiếu |
|---|---|---|
| frontend_web | | stack-packs/frontend/<x>.md |
| mobile | | stack-packs/mobile/<x>.md |
| backend | | stack-packs/backend/<x>.md |
| infrastructure | | stack-packs/deploy/<x>.md |

## Data model
<Schema mới/thay đổi — bảng, quan hệ, migration cần thiết>

## Source tree / module boundaries
<Thư mục nào bị ảnh hưởng, module mới nào được tạo>

## Architecture Decision Records liên quan
<Liệt kê ADR mới cần tạo trong templates/decisions/ cho các lựa chọn lớn — không lặp lại
nội dung ADR ở đây, chỉ trỏ tới>

## Rủi ro kỹ thuật & phương án giảm thiểu
| Rủi ro | Mức độ | Giảm thiểu |
|---|---|---|
| | | |

## Câu trả lời cho câu hỏi mở trong PRD
<Trả lời trực tiếp từng câu hỏi PRD đã nêu — nếu còn câu chưa trả lời được, Solutioning
Gate không thể PASS>

## Đề xuất tách story packet
<Danh sách story packet dự kiến (id tạm), mỗi story nên đủ nhỏ để review theo lane
tương ứng trong core/HARNESS.md>
