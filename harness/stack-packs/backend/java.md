# Stack Pack: Backend — Java (Spring Boot)

> Nạp file này khi `harness.config.yaml → surfaces.backend.stack: java-spring-boot`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
API backend bằng Java 17+/Spring Boot.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Test | `./mvnw test` |
| Build | `./mvnw package -DskipTests -q` |
| Run | `./mvnw spring-boot:run` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Repository layer trả `Optional<T>` — không trả `null`.
- `@Transactional` ở service layer — không đặt ở repository.
- Không nối chuỗi vào JPQL/SQL — dùng parameter binding.
- Migration mới = file Flyway/Liquibase mới — không sửa migration đã chạy.
- External API call qua circuit breaker (Resilience4j) — không gọi trực tiếp không timeout.

## Lỗi thường gặp agent hay mắc (tránh)
- N+1 query — bật `hibernate.generate_statistics=true` khi test để phát hiện.
- LazyInitializationException do truy cập lazy collection ngoài transaction.

## Quality gate bổ sung
Xem `quality-gates/backend-gates.md`.

## Tham chiếu
- https://docs.spring.io/spring-boot/
