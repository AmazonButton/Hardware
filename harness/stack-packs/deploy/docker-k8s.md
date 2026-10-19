# Stack Pack: Deploy — Docker + Kubernetes

> Nạp file này khi `harness.config.yaml → surfaces.infrastructure.stack: docker-k8s`.
> Không nạp song song nhiều stack-pack cùng nhóm (vd 2 backend) cho cùng 1 surface — mỗi surface chỉ có 1 stack.

## Khi nào dùng
Deploy self-managed hoặc managed K8s (EKS/GKE/AKS), container hoá bằng Docker.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Build image | `docker build -t <image>:<tag> .` |
| Scan image | `trivy image <image>:<tag>` |
| Apply manifest | `kubectl apply -f k8s/ --dry-run=client` |
| Rollout status | `kubectl rollout status deploy/<name>` |

## Quy ước bắt buộc (agent PHẢI tuân theo)
- Multi-stage Dockerfile — image production không chứa build tool/devDependencies.
- Non-root user trong container (`USER` directive) — không chạy container as root trừ lý do rõ ràng.
- Resource `requests`/`limits` bắt buộc trong mọi Deployment manifest.
- Readiness/liveness probe bắt buộc — không deploy service không có health check.
- Secrets qua K8s Secret/External Secrets Operator — không bake vào image hay env plaintext trong manifest commit.

## Lỗi thường gặp agent hay mắc (tránh)
- Rolling update không có `PodDisruptionBudget` → downtime khi node drain.
- Image tag `:latest` trong production — luôn pin version/digest.
- Quên `imagePullPolicy` phù hợp gây deploy nhầm image cũ do cache.

## Quality gate bổ sung
Security scan image bắt buộc trước khi push registry (xem `workflows/reusable/security-scan.yml`).

## Tham chiếu
- https://kubernetes.io/docs/concepts/
- workflows/reusable/harness-quality-gate.yml
