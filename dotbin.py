"""dotbin.py - 生成 AT27C64 EPROM 二进制图案文件

8x8 LED 点阵字模生成工具。每个图案占 8 字节（每字节一行），
最多存储 8 幅图案（共 64 字节），对应地址线 A3/A4/A5 选图案。

用法:
    python dotbin.py                    # 使用内置示例图案生成 pattern.bin
    python dotbin.py -o output.bin      # 指定输出文件名
    python dotbin.py -s                 # 在终端预览所有图案
    python dotbin.py -p 0               # 只预览第 0 幅图案

地址映射:
    图案0 -> 0x00~0x07, 图案1 -> 0x08~0x0F, ..., 图案7 -> 0x38~0x3F
"""

import struct
import sys
import os

# ═══════════════════════════════════════════════════════════
#  图案数据区：每幅图案 8 字节，可自由修改
#  1 = LED 亮, 0 = LED 灭  (bit 7=最左列C7, bit 0=最右列C0)
# ═══════════════════════════════════════════════════════════

PATTERNS = [
    # 图案 0 — 爱心
    [0xFF, 0x99, 0x66, 0x42, 0x24, 0x18, 0x00, 0x00],

    # 图案 1 — 笑脸
    [0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C],

    # 图案 2 — 哭脸
    [0x3C, 0x42, 0xA5, 0x81, 0x99, 0xA5, 0x42, 0x3C],

    # 图案 3 — 全亮（测试用）
    [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF],

    # 图案 4 — 十字
    [0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81],

    # 图案 5 — 棋盘格
    [0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55],

    # 图案 6 — 边框
    [0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF],

    # 图案 7 — 全灭（空白）
    [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
]

MAX_PATTERNS = 8
BYTES_PER_PATTERN = 8


def draw_pattern(pattern: list[int], index: int = 0):
    """在终端可视化打印一幅 8x8 图案"""
    name = ""
    names = ["爱心", "笑脸", "哭脸", "全亮", "十字", "棋盘格", "边框", "全灭"]
    if index < len(names):
        name = names[index]
    print(f"\n  图案 {index} — {name}")
    print("  " + "-" * 17)
    for row_byte in pattern:
        line = "  |"
        for col in range(8):
            bit = (row_byte >> (7 - col)) & 1
            line += "■ " if bit else "  "
        line += "|"
        print(line)
    print("  " + "-" * 17)
    hex_str = ", ".join(f"0x{b:02X}" for b in pattern)
    print(f"  字模: {{{hex_str}}}")


def draw_all(patterns: list[list[int]]):
    """预览所有图案"""
    for i, pat in enumerate(patterns):
        draw_pattern(pat, i)


def validate_pattern(pattern: list[int]) -> bool:
    """检查图案是否合法"""
    if len(pattern) != 8:
        return False
    return all(0 <= b <= 0xFF for b in pattern)


def patterns_to_bytes(patterns: list[list[int]]) -> bytes:
    """将图案列表转为二进制字节"""
    buf = bytearray()
    for i in range(MAX_PATTERNS):
        if i < len(patterns):
            pat = patterns[i]
            if not validate_pattern(pat):
                print(f"错误: 图案 {i} 格式不正确（需要 8 字节，每字节 0~255）")
                sys.exit(1)
            buf.extend(pat)
        else:
            buf.extend([0xFF] * BYTES_PER_PATTERN)  # 未定义图案填充全亮
    return bytes(buf)


def write_bin(data: bytes, path: str):
    """写入 .bin 文件"""
    with open(path, "wb") as f:
        f.write(data)
    print(f"已生成: {os.path.abspath(path)}  ({len(data)} 字节)")


def main():
    output = "pattern.bin"
    preview = False
    preview_pattern = None

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "-o" and i + 1 < len(args):
            output = args[i + 1]
            i += 2
        elif args[i] == "-s":
            preview = True
            i += 1
        elif args[i] == "-p" and i + 1 < len(args):
            preview = True
            preview_pattern = int(args[i + 1])
            i += 2
        elif args[i] in ("-h", "--help"):
            print(__doc__)
            return
        else:
            i += 1

    if not validate_pattern(PATTERNS[0]):
        print("错误: 图案数据不合法")
        sys.exit(1)

    if preview:
        if preview_pattern is not None:
            if 0 <= preview_pattern < len(PATTERNS):
                draw_pattern(PATTERNS[preview_pattern], preview_pattern)
            else:
                print(f"图案索引 {preview_pattern} 超出范围 (0~{len(PATTERNS)-1})")
        else:
            draw_all(PATTERNS)
        return

    data = patterns_to_bytes(PATTERNS)
    write_bin(data, output)


if __name__ == "__main__":
    main()
