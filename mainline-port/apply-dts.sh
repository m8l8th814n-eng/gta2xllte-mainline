#!/usr/bin/env bash
#
# apply-dts.sh — applicera den redigerade dts:en i kärnträdet och verifiera.
#
# Arbetsflöde:
#   1. redigera/spara:  mainline-port/msm8953-samsung-gta2xlwifi.dts   (DENNA är källan)
#   2. kör:             ./apply-dts.sh
#   3. bygg:            pmbootstrap build --src=<kärnträd> linux-postmarketos-qcom-msm8953
#
set -euo pipefail

DTS_NAME="msm8953-samsung-gta2xlwifi"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANONICAL="$HERE/$DTS_NAME.dts"
KTREE="${1:-/home/simon/pmos/mainline-build/linux-7.0.9-r0}"
QCOM="$KTREE/arch/arm64/boot/dts/qcom"

[ -f "$CANONICAL" ] || { echo "FEL: hittar inte $CANONICAL" >&2; exit 1; }
[ -d "$QCOM" ]      || { echo "FEL: kärnträd saknas: $QCOM" >&2; exit 1; }

# 1. kopiera in dts:en
cp "$CANONICAL" "$QCOM/$DTS_NAME.dts"
echo "kopierad -> $QCOM/$DTS_NAME.dts"

# 2. se till att Makefile-raden finns
if ! grep -q "$DTS_NAME.dtb" "$QCOM/Makefile"; then
	# lägg den alfabetiskt efter motorola-potter om möjligt, annars sist
	if grep -q "msm8953-motorola-potter.dtb" "$QCOM/Makefile"; then
		sed -i "/msm8953-motorola-potter.dtb/a dtb-\$(CONFIG_ARCH_QCOM)\t+= $DTS_NAME.dtb" "$QCOM/Makefile"
	else
		echo "dtb-\$(CONFIG_ARCH_QCOM) += $DTS_NAME.dtb" >> "$QCOM/Makefile"
	fi
	echo "lade till Makefile-rad för $DTS_NAME.dtb"
fi

# 3. verifiera att den kompilerar (host dtc, snabbt)
echo "=== kompilerar dtb (verifiering) ==="
make -C "$KTREE" ARCH=arm64 W=1 "qcom/$DTS_NAME.dtb"
echo
echo "OK — $QCOM/$DTS_NAME.dtb byggd. Redo för pmbootstrap build."
