# Stack Pack: Frontend — Vue 3 (Vite/Nuxt)

> Nạp file này khi `harness.config.yaml → surfaces.frontend_web.stack: vue3-vite-typescript | nuxt`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Web app dùng Vue 3 Composition API, TypeScript.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| TypeCheck | `vue-tsc --noEmit` |
| Lint | `npm run lint` |
| Unit test | `npm run test:unit -- --run` |
| Build | `npm run build` |
| Dev | `npm run dev` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Luôn dùng `<script setup lang="ts">` — tránh Options API trong code mới.
- State chia sẻ qua Pinia store — không dùng `provide/inject` cho global state phức tạp.
- Props/emit định nghĩa type rõ ràng bằng `defineProps<T>()`/`defineEmits<T>()`.
- Nuxt: phân biệt rõ code chạy server-only (`server/`) vs universal (`composables/`).

## Lỗi thường gặp agent hay mắc (tránh)
- Mất reactivity khi destructure props không dùng `toRefs`.
- Watcher không có `{ deep: true }` khi theo dõi object lồng nhau nhưng cần.
- SSR: truy cập `window`/`document` ngoài `onMounted`/`import.meta.client`.

## Quality gate bổ sung
Xem `quality-gates/frontend-gates.md`.

## Tham chiếu
- https://vuejs.org
- https://nuxt.com/docs
