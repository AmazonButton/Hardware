# Stack Pack: Deploy — Static/Edge (Vercel/Netlify/Cloudflare Pages)

> Nạp file này khi `harness.config.yaml → surfaces.infrastructure.stack: vercel | netlify | cloudflare-pages`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Frontend/SSR deploy lên platform edge/hosting quản lý (không tự quản K8s/VM).

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Preview deploy | `vercel  # hoặc: netlify deploy` |
| Production deploy | `vercel --prod  # hoặc: netlify deploy --prod` |
| Env check | `vercel env ls  # hoặc: netlify env:list` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Preview deployment bắt buộc cho mọi PR trước khi merge lên nhánh production.
- Biến môi trường nhạy cảm chỉ set qua dashboard/CLI platform — không commit `.env` thật.
- Edge function: tránh Node-only API không tương thích runtime edge (kiểm tra compatibility trước khi dùng).
- Cache-Control header rõ ràng cho asset tĩnh — tránh cache mặc định gây stale content sau deploy.

## Lỗi thường gặp agent hay mắc (tránh)
- Redirect/rewrite rule không test kỹ gây vòng lặp redirect trên production.
- Build thành công ở local nhưng fail trên platform do khác Node version — pin version trong config.

## Quality gate bổ sung
Preview URL phải được QA Engineer/role review kiểm tra trước khi approve production deploy.

## Tham chiếu
- https://vercel.com/docs
- https://docs.netlify.com
