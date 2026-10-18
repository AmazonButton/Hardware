# Domain Pack: E-Commerce

> Nạp thêm khi `project.domain: "ecommerce"`. Cộng thêm vào `compliance/COMPLIANCE_FRAMEWORK.md`.

## 1. Bối cảnh
Sàn TMĐT, marketplace, giỏ hàng, thanh toán tiêu dùng. Chịu thêm ràng buộc luật bảo vệ người tiêu dùng, quảng cáo giá, và chính sách chống dark-pattern của các nền tảng lớn.

## 2. Nguyên tắc giá & khuyến mãi
- Giá "gạch ngang"/giảm giá hiển thị phải phản ánh đúng giá đã bán trước đó trong khoảng thời gian hợp lý — không tạo giá ảo để tạo cảm giác giảm giá giả.
- Không dùng đồng hồ đếm ngược giả hoặc thông báo "chỉ còn X sản phẩm" nếu không đúng số liệu thực từ hệ thống kho.

## 3. Không dark-pattern
- Nút huỷ đơn/huỷ đăng ký (subscription) phải dễ tìm và số bước tương đương với nút đăng ký.
- Không tự động thêm sản phẩm/dịch vụ vào giỏ hàng mà không có hành động rõ ràng của người dùng.

## 4. Dữ liệu & review
- Review sản phẩm hiển thị phải phân biệt rõ review thật (có mua hàng) và review chưa xác minh nếu hệ thống có 2 loại.
- Dữ liệu thanh toán tuân theo PCI-DSS ở mức tối thiểu cần thiết (khuyến nghị không tự lưu số thẻ, dùng payment gateway tokenization).

## 5. Cổng kiểm tra tự động bổ sung
- CI kiểm tra mọi trang sản phẩm có hiển thị giá/khuyến mãi phải có test snapshot xác nhận không có yếu tố đếm ngược/số lượng giả cứng trong code (hardcode).
