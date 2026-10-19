# Stack Pack: Mobile — Unity 2D (C# / URP)

> Nạp file này khi `harness.config.yaml → surfaces.mobile.stack: unity-2d-csharp`.
> Áp dụng cho Game Client 2D ARPG Anhem JaeBin.

## Khi nào dùng
Dự án game 2D ARPG mobile đa nền tảng (Android APK / iOS) phát triển bằng Unity Engine (URP) và ngôn ngữ C#.

## Lệnh chuẩn (điền vào surface `AGENTS.md`)
| Action | Command |
|---|---|
| Build Android Debug | `& "C:\Program Files\Unity\Hub\Editor\<version>\Editor\Unity.exe" -batchmode -quit -projectPath . -executeMethod BuildScript.BuildAndroidDebug -logFile build.log` |
| Run Tests (NUnit) | `& "C:\Program Files\Unity\Hub\Editor\<version>\Editor\Unity.exe" -batchmode -runTests -projectPath . -testPlatform EditMode -testResults results.xml` |
| Format / Lint | `dotnet format` (hoặc C# analyzers tích hợp trong IDE) |

## Quy ước bắt buộc (Agent PHẢI tuân theo)
1. **Tách biệt Logic và Render (Pure C# vs MonoBehaviour)**:
   - Các logic tính toán chỉ số, sát thương, tỉ lệ drop phải viết trong các class Pure C# (`CombatCalculator.cs`, `StatSystem.cs`), có Unit Test độc lập không phụ thuộc Unity Lifecycle.
   - MonoBehaviour chỉ làm nhiệm vụ tiếp nhận Input và điều khiển Render/Animation/VFX (`PlayerView.cs`, `CombatPresenter.cs`).
2. **Quản lý Bộ nhớ & GC Spike (Zero Allocation in Update)**:
   - Tuyệt đối không `new` object, không dùng `LINQ`, không ghép chuỗi string (dùng `StringBuilder` hoặc cache) bên trong hàm `Update()` hoặc `FixedUpdate()`.
   - Bắt buộc dùng **Object Pooling** cho đạn, quái lâu la, hiệu ứng VFX chém trúng và số nhảy sát thương (Damage Popup).
3. **Quản lý Tài nguyên (Addressables / Asset Bundles)**:
   - Không nhét toàn bộ asset dung lượng lớn vào thư mục `Resources/`.
   - Sử dụng Addressable Asset System cho skin, audio và map để giảm kích thước gói APK ban đầu dưới 100MB.
4. **Mobile Target 60 FPS**:
   - Tối ưu Sprite Atlases, hạn chế Draw Calls bằng cách gộp texture.
   - Cấu hình URP 2D Renderer với Render Scale phù hợp (0.85 - 1.0) cho thiết bị tầm trung.

## Lỗi thường gặp agent hay mắc (Tránh)
- Sử dụng `Find()`, `FindObjectOfType()` hoặc `GetComponent()` lặp đi lặp lại trong vòng lặp `Update()`.
- Bỏ qua cấu hình VSync / Frame rate lock (`Application.targetFrameRate = 60`) khiến điện thoại bị quá nhiệt và ngốn pin.
- Không xử lý đa độ phân giải màn hình (tai thỏ, nốt ruồi notch trên Android/iOS) cho giao diện Canvas UI.

## Quality gate bổ sung
Xem `quality-gates/mobile-gates.md`.
