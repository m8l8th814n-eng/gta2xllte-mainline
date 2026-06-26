# postmarketOS mainline — Samsung Galaxy Tab A 10.5 (2018, Wi-Fi)

Mainline port of **gta2xlwifi** (SoC Qualcomm SDM450 / MSM8953).

Base: pmOS device profile `qcom-msm8953` (systemd-edge), kernel
`linux-postmarketos-qcom-msm8953` (msm8953-mainline/linux v7.0.9-r0), bootloader lk2nd.

## Status

| Feature | State |
|---|---|
| Boot (kernel + dtb) | ✅ |
| eMMC (`mmcblk1`) | ✅ |
| Userspace + ssh | ✅ |
| Display | ✅ simpledrm on lk2nd's framebuffer |
| Wi-Fi | ✅ wcn36xx (WCNSS pronto + WCN3680 iris) |
| Bluetooth | ✅ (WCNSS) |
| Touch (ST FTS) | 🟡 chip responds, `stmfts` times out (`-110`, fts1ba90a) |
| DSI panel (HX8279) | ⛔ deferred — simpledrm used instead |
| Modem | disabled (Wi-Fi variant) |
| Sensors, camera, audio, GPU | not started |

## Layout

```
mainline-port/
├── msm8953-samsung-gta2xlwifi.dts   # device tree (the source)
├── apply-dts.sh                     # copy dts → kernel tree + verify (out-of-tree)
├── build-bootimg.sh                 # build a RAM-bootable boot.img from the kernel apk
├── pmaports-overlay/                # kernel config changes + diff
├── downstream-dts/                  # downstream Android dts (hardware reference)
├── templates/                       # mainline msm8953 dts (daisy, mido)
└── reference/live-fdt-downstream.dts# decompiled /sys/firmware/fdt
```

The kernel tree (`~/pmos/mainline-build/linux-7.0.9-r0`) lives outside this repo.

## Workflow

```bash
cd mainline-port
# edit msm8953-samsung-gta2xlwifi.dts
./apply-dts.sh                       # into the kernel tree + verify (always run before build)
./build-bootimg.sh                   # dts change → images/boot.img
# config/kernel change: rebuild the kernel first:
#   pmbootstrap build --src=$HOME/pmos/mainline-build/linux-7.0.9-r0 linux-postmarketos-qcom-msm8953
fastboot boot images/boot.img        # RAM boot (nothing is flashed)
```

## Notes

- **PMIC:** pm8953 only (no pmi8950/pmi8937). Do not include `pmi8950.dtsi` — the SPMI
  probe fails with -EIO and blocks everything behind it.
- **dtb selection:** lk2nd matches `qcom,msm-id = <0x152 0x0>` + `qcom,board-id = <0x08 0x04>`.
  Without them the downstream dtb is loaded.
- **Display:** no mainline path lights the HX8279 (the ISL98608 has no i2c driver). Use
  simpledrm on the cont_splash framebuffer at `0x90001000`, msm DSI disabled, panel rails
  always-on. Format: 1200×1920, stride 1200×3, `r8g8b8` (24bpp) — 32bpp gives a sheared image.
- **Touch:** i2c_1 (`78b5000`) uses gpio 2–3, so `gpio-reserved-ranges = <0 2>` (not daisy's
  `<0 4>`). Driver `CONFIG_TOUCHSCREEN_STMFTS`. The panel uses an ST fts1ba90a, which the
  mainline stmfts driver likely does not speak.
- **Wi-Fi/BT:** WCNSS pronto + iris `qcom,wcn3680` (not `wcn3680b` — that compatible
  doesn't exist). Needs the device NV blob at the `firmware-name` path
  `qcom/msm8953/samsung/gta2xlwifi/WCNSS_qcom_wlan_nv.bin` (32K, from the stock
  firmware) or wcn36xx fails with -2.
- **Root UUIDs:** root `07d2abef-9505-4fc9-a6da-b6e69594ae30`,
  boot `1298aa1d-1977-4172-a50b-d7aead5d4569` (install on `mmcblk1p47`).
- **Build hygiene:** do not run `make` against the kernel tree on the Arch host — host
  gcc/glibc pollute the config (`fixdep`) and break the musl chroot build. The scripts build
  out-of-tree; config changes are edited by hand.

## TODO

- Touch: verify/adapt a driver for fts1ba90a.
- DSI panel: ISL98608 i2c config, then `&mdss`/`&mdss_dsi0` instead of simpledrm.
- Package the Wi-Fi NV firmware so it survives a reinstall.
- GPU (Adreno 506, `a506_zap`), sensors, audio, charger.
- Spontaneous reboots (likely watchdog).

## Upstream

DTS submitted to msm8953-mainline/linux: PR #253.
