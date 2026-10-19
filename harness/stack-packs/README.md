# Stack Packs

> **Mục đích**: tách "quy trình harness" (roles, gates, lanes, review) — vốn giống nhau ở mọi
> dự án — khỏi "quy ước theo công nghệ cụ thể" (React vs Vue, Node vs Go, Docker vs Serverless…).
> Đây chính là cơ chế giúp bộ harness này **dùng lại được cho bất kỳ stack nào** thay vì
> khoá cứng vào 1 bộ công nghệ (vấn đề v0.2.0 từng mắc: hardcode Java Spring Boot + Expo
> ngay trong `HIERARCHICAL_AGENTS.md`).

## Cách hoạt động

1. Trong `harness.config.yaml`, mỗi surface khai báo `stack:` (vd `stack: "react-vite-typescript"`).
2. Agent (Orchestrator hoặc surface-level agent) đọc `harness.config.yaml`, map `stack` →
   file tương ứng trong `stack-packs/<nhóm>/<stack>.md`, và nạp file đó vào context **chỉ khi
   làm việc trên surface đó** (giống cách `compliance/domain-packs/*.md` được nạp theo
   `project.domain`).
3. Surface `AGENTS.md` (`[dir]/AGENTS.md`) nên có 1 dòng trỏ thẳng tới stack-pack tương ứng
   thay vì copy-paste nội dung, để tránh trôi (drift) giữa 2 nguồn.

```markdown
## Stack
Xem stack-pack: harness-engineering/stack-packs/backend/node.md
```

## Danh mục

| Nhóm | File | stack_id gợi ý trong config |
|---|---|---|
| Frontend | `frontend/react.md` | `react-vite-typescript`, `nextjs` |
| Frontend | `frontend/vue.md` | `vue3-vite-typescript`, `nuxt` |
| Frontend | `frontend/svelte.md` | `svelte5-sveltekit` |
| Mobile | `mobile/react-native.md` | `expo-react-native` |
| Mobile | `mobile/flutter.md` | `flutter` |
| Mobile | `mobile/native-ios.md` | `native-ios-swiftui` |
| Mobile | `mobile/native-android.md` | `native-android-compose` |
| Backend | `backend/node.md` | `node-nestjs`, `node-express`, `node-fastify` |
| Backend | `backend/python.md` | `python-fastapi`, `python-django` |
| Backend | `backend/go.md` | `go-gin`, `go-echo`, `go-stdlib` |
| Backend | `backend/java.md` | `java-spring-boot` |
| Deploy | `deploy/docker-k8s.md` | `docker-k8s` |
| Deploy | `deploy/serverless.md` | `serverless-lambda`, `cloudflare-workers` |
| Deploy | `deploy/static-edge.md` | `vercel`, `netlify`, `cloudflare-pages` |
| Deploy | `deploy/mobile-store-release.md` | `play-store`, `app-store` |

## Chưa có stack bạn cần?

Copy `stack-packs/_TEMPLATE.md`, điền 6 mục (Khi nào dùng / Lệnh chuẩn / Quy ước bắt buộc /
Lỗi thường gặp / Quality gate bổ sung / Tham chiếu), đặt vào đúng nhóm. Giữ file **ngắn
(< 60 dòng)** — đây là adapter nạp theo nhu cầu, không phải tài liệu học công nghệ.

Nếu bạn maintain 1 stack không có trong danh mục và đã dùng ổn định, cân nhắc gửi PR bổ
sung — mục tiêu là danh mục này phủ được phần lớn dự án web/mobile/backend/deploy phổ biến.
