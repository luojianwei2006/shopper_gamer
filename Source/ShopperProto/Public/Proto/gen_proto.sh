#!/usr/bin/env bash
#
# gen_proto.sh — 把 .proto 生成 UE5 可编译的 protobuf C++ 代码
#
# 解决 UE5 + 原生 protobuf 的三大坑：
#   1. protoc 默认生成 *.pb.cc，而 UBT 只编译 *.cpp  -> 改名
#   2. UE 的 PCH 把 verify 定义成宏，与 abseil btree.h 的 verify() 成员冲突
#      -> 每个 .pb.cpp 顶部加 #undef verify（必须在任何 protobuf/abseil include 之前）
#   3. protoc 用错 --proto_path 会写出 "Source/.../X.pb.h" 这类工程相对/绝对路径
#      -> 收敛为相对文件名 "X.pb.h"（UBT 按 quote-include 从 .cpp 同目录查找）
#
# 用法：
#   ./gen_proto.sh                     # 处理本目录所有 .proto
#   PROTOC=/path/to/protoc ./gen_proto.sh
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

PROTOC="${PROTOC:-protoc}"
if ! command -v "$PROTOC" >/dev/null 2>&1; then
  echo "[gen_proto] ERROR: protoc 未找到，请用 PROTOC=/path/to/protoc 指定" >&2
  exit 1
fi

echo "[gen_proto] using: $(command -v "$PROTOC") ($("$PROTOC" --version 2>&1))"

shopt -s nullglob
PROTO_FILES=( *.proto )
shopt -u nullglob

if [ ${#PROTO_FILES[@]} -eq 0 ]; then
  echo "[gen_proto] 本目录没有 .proto 文件，退出"
  exit 0
fi

# 1) 生成 .pb.cc / .pb.h（--proto_path=. 保证自引用为相对路径）
echo "[gen_proto] generating from: ${PROTO_FILES[*]}"
"$PROTOC" --proto_path="." --cpp_out="." "${PROTO_FILES[@]}"

# 2) 改名 .pb.cc -> .pb.cpp（UBT 只认 .cpp）
for cc in *.pb.cc; do
  [ -e "$cc" ] || continue
  cpp="${cc%.cc}.cpp"
  mv "$cc" "$cpp"
  echo "[gen_proto] renamed $cc -> $cpp"
done

# 3) 给每个 .pb.cpp 打补丁（幂等，可重复执行）
for cpp in *.pb.cpp; do
  [ -e "$cpp" ] || continue
  base="$(basename "$cpp" .cpp)"   # 例: Sample.pb

  # 3a) 幂等插入 #undef verify（仅在尚未存在时）
  if ! grep -qE '^#undef verify([[:space:]]|$)' "$cpp"; then
    tmp="$(mktemp)"
    { echo '#undef verify'; cat "$cpp"; } > "$tmp"
    mv "$tmp" "$cpp"
    echo "[gen_proto] + #undef verify -> $cpp"
  fi

  # 3b) 收敛自引用 include 路径：".../X.pb.h" -> "X.pb.h"
  if grep -q "#include \".*/${base}.h\"" "$cpp"; then
    sed -i '' "s|#include \".*/${base}.h\"|#include \"${base}.h\"|" "$cpp"
    echo "[gen_proto] fixed self-include -> $cpp"
  fi
done

echo "[gen_proto] done."
