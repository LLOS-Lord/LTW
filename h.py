#!/usr/bin/env python3
"""
Đẩy toàn bộ thư mục lên GitHub.
Các file > 90 MB sẽ được tự động chia nhỏ để tránh giới hạn của GitHub.
Sử dụng: python3 push_folder.py <đường_dẫn_thư_mục> [-m "commit message"]
"""

import sys
import subprocess
import os
import math

CHUNK_SIZE = 90 * 1024 * 1024  # 90 MB, an toàn dưới giới hạn GitHub (100 MB)

def run_cmd(cmd):
    """
    Chạy lệnh shell, hiển thị đầy đủ stdout và stderr nếu lỗi, rồi thoát.
    """
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"\n❌ LỖI khi chạy: {cmd}")
        stdout = result.stdout.strip()
        stderr = result.stderr.strip()
        if stdout:
            print(f"stdout:\n{stdout}")
        if stderr:
            print(f"stderr:\n{stderr}")
        sys.exit(1)
    return result.stdout.strip()

def split_file(file_path):
    """
    Chia file lớn thành các phần ≤ CHUNK_SIZE.
    Trả về danh sách đường dẫn các phần và file README hướng dẫn.
    """
    prefix = file_path + ".part"
    # Xóa các phần cũ (nếu có) để tránh lẫn lộn
    subprocess.run(f"rm -f '{prefix}'*", shell=True)

    print(f"  🔪 Đang chia {file_path}...")
    run_cmd(f"split -b {CHUNK_SIZE} -d '{file_path}' '{prefix}'")

    # Lấy danh sách phần đã tạo
    dir_name = os.path.dirname(file_path) or "."
    base_prefix = os.path.basename(prefix)
    all_files = os.listdir(dir_name)
    parts = sorted([f for f in all_files if f.startswith(base_prefix)])

    # Tạo file README hướng dẫn ghép
    readme_name = file_path + ".README.txt"
    with open(readme_name, 'w', encoding='utf-8') as f:
        f.write(f"File gốc: {os.path.basename(file_path)}\n")
        f.write(f"Được chia thành {len(parts)} phần.\n")
        f.write("Để ghép lại, chạy lệnh:\n")
        f.write(f"  cat {base_prefix}* > {os.path.basename(file_path)}\n")
        f.write("Sau đó có thể xóa các file .part* và file README này.\n")

    full_parts = [os.path.join(dir_name, p) for p in parts]
    return full_parts + [readme_name]

def main():
    if len(sys.argv) < 2:
        print("Sử dụng: python3 push_folder.py <thư_mục> [-m \"message\"]")
        print("Ví dụ: python3 push_folder.py project -m \"Thêm project\"")
        sys.exit(1)

    folder = sys.argv[1]
    if not os.path.isdir(folder):
        print(f"'{folder}' không tồn tại hoặc không phải thư mục.")
        sys.exit(1)

    # Lấy commit message (nếu có)
    commit_msg = None
    if "-m" in sys.argv:
        try:
            idx = sys.argv.index("-m")
            commit_msg = sys.argv[idx + 1]
        except (ValueError, IndexError):
            pass
    if not commit_msg:
        commit_msg = f"Đẩy thư mục {folder}"

    # Kiểm tra đang trong git repo
    if not os.path.isdir(".git"):
        print("Bạn chưa ở trong một Git repository. Hãy cd vào repo trước.")
        sys.exit(1)

    # Thu thập tất cả file cần push (bỏ qua thư mục .git)
    normal_files = []        # file ≤ 90 MB
    large_files_parts = []   # các phần của file lớn + file README

    print("🔍 Đang quét thư mục...")
    for root, dirs, files in os.walk(folder):
        # Không duyệt vào .git
        if '.git' in dirs:
            dirs.remove('.git')

        for file in files:
            file_path = os.path.join(root, file)

            # Bỏ qua các file .part và .README.txt do script tạo ra
            if file.endswith(".part") or file.endswith(".README.txt"):
                base_name = file
                if file.endswith(".README.txt"):
                    base_name = file[:-len(".README.txt")]
                else:
                    idx_part = file.rfind(".part")
                    if idx_part != -1:
                        base_name = file[:idx_part]
                original = os.path.join(root, base_name)
                if os.path.exists(original):
                    continue

            file_size = os.path.getsize(file_path)
            if file_size >= CHUNK_SIZE:
                print(f" ⚠️  Phát hiện file lớn: {file_path} ({file_size/1024/1024:.1f} MB)")
                parts = split_file(file_path)
                large_files_parts.extend(parts)
            else:
                normal_files.append(file_path)

    if not normal_files and not large_files_parts:
        print("✅ Không có file mới hoặc thay đổi nào trong thư mục (đã được đẩy trước đó).")
        sys.exit(0)

    # Stage tất cả
    print("\n📦 Đang stage các file...")
    for f in normal_files:
        run_cmd(f"git add '{f}'")
    for f in large_files_parts:
        run_cmd(f"git add '{f}'")

    # Kiểm tra xem có thay đổi nào đã được stage hay không
    staged = run_cmd("git diff --cached --name-only")
    if not staged.strip():
        print("✅ Không có thay đổi nào để commit (các file đã up-to-date hoặc không khác gì phiên bản cũ).")
        sys.exit(0)

    # Commit
    print(f"\n💾 Commit với message: \"{commit_msg}\"")
    run_cmd(f'git commit -m "{commit_msg}"')

    # Push
    print("🚀 Đang push lên GitHub...")
    run_cmd("git push")

    print("\n✅ Thành công! Thư mục đã được đẩy lên repo.")

if __name__ == "__main__":
    main()