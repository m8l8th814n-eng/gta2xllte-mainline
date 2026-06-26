# postmarketOS mainline — Samsung Galaxy Tab A 10.5 (2018, Wi-Fi)

Mainline-port av **gta2xlwifi** (SoC Qualcomm SDM450 / MSM8953).

Bas: pmOS device-profil `qcom-msm8953` (systemd-edge), kärna
`linux-postmarketos-qcom-msm8953` (msm8953-mainline/linux v7.0.9-r0), bootloader lk2nd.

## Status

| Funktion | Läge |
|---|---|
| Boot (kärna + dtb) | ✅ |
| eMMC (`mmcblk1`) | ✅ |
| Userspace + ssh | ✅ |
| Display | ✅ simpledrm på lk2nd:s framebuffer |
| Touch (ST FTS) | 🟡 chipet svarar, `stmfts` timeoutar (`-110`) |
| DSI-panel (HX8279) | ⛔ uppskjuten — simpledrm används |
| Modem | disablat (Wi-Fi-variant) |
| Wi-Fi/BT, sensorer, kamera, ljud, GPU | ej påbörjat |

## Struktur

```
mainline-port/
├── msm8953-samsung-gta2xlwifi.dts   # device tree (källan)
├── apply-dts.sh                     # kopiera dts → kärnträd + verifiera (out-of-tree)
├── build-bootimg.sh                 # bygg RAM-bootbar boot.img ur kärn-apk:n
├── pmaports-overlay/                # kärn-config-ändringar + diff
├── downstream-dts/                  # nedströms Android-dts (referens)
├── templates/                       # mainline msm8953-dts (daisy, mido)
└── reference/live-fdt-downstream.dts# dekompilerad /sys/firmware/fdt
```

Kärnträdet (`~/pmos/mainline-build/linux-7.0.9-r0`) ligger utanför repot.

## Arbetsflöde

```bash
cd mainline-port
# redigera msm8953-samsung-gta2xlwifi.dts
./apply-dts.sh                       # in i kärnträdet + verifiera (kör alltid före build)
./build-bootimg.sh                   # dts-ändring → images/boot.img
# config-/kärnändring: bygg om kärnan först:
#   pmbootstrap build --src=$HOME/pmos/mainline-build/linux-7.0.9-r0 linux-postmarketos-qcom-msm8953
fastboot boot images/boot.img        # RAM-boot (inget flashas)
```

## Tekniska noter

- **PMIC:** endast pm8953 (ingen pmi8950/pmi8937). `pmi8950.dtsi` får inte inkluderas
  — SPMI-probet failar -EIO och blockerar allt bakom.
- **dtb-val:** lk2nd matchar `qcom,msm-id = <0x152 0x0>` + `qcom,board-id = <0x08 0x04>`.
  Utan dem laddas downstream-dtb:n.
- **Display:** ingen mainline-väg tänder HX8279 (ISL98608 saknar i2c-driver). simpledrm
  på cont_splash-framebuffer `0x90001000`, msm-DSI disablad, panel-rails always-on.
  Format: 1200×1920, stride 1200×3, `r8g8b8` (24bpp) — 32bpp ger skev bild.
- **Touch:** i2c_1 (`78b5000`) använder gpio 2–3 → `gpio-reserved-ranges = <0 2>`
  (inte daisys `<0 4>`). Driver `CONFIG_TOUCHSCREEN_STMFTS`. Panelen är ST fts1ba90a,
  som mainline-stmfts troligen inte talar med.
- **Root-UUID:** root `07d2abef-9505-4fc9-a6da-b6e69594ae30`,
  boot `1298aa1d-1977-4172-a50b-d7aead5d4569` (install på `mmcblk1p47`).
- **Bygg-hygien:** kör inte `make` mot kärnträdet på Arch-värden — host-gcc/glibc
  förorenar configen (`fixdep`) och bryter musl-chroot-bygget. Scripten bygger
  out-of-tree; config-ändringar görs för hand.

## Kvar

- Touch: verifiera/anpassa driver för fts1ba90a.
- DSI-panel: ISL98608 i2c-config, sen `&mdss`/`&mdss_dsi0` i stället för simpledrm.
- GPU (Adreno 506, `a506_zap`), Wi-Fi/BT (`&wcnss`), sensorer, ljud, laddare.
- Spontana reboots (trolig watchdog).
