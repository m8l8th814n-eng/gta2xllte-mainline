# postmarketOS mainline — Samsung Galaxy Tab A 10.5 (2018, LTE)

I have stopped trying get this device into mainline. but I have another repo with patches for this device that seems to work a lot better.
It has msmdrm instead of simpledrm as this repo has.
I only made it to learn more about qualcomms.


Mainline port of **gta2xllte** (SM-T595), SoC Qualcomm SDM450.

Base: pmOS device profile `qcom-msm8953` (systemd-edge), kernel
`linux-postmarketos-qcom-msm8953` (msm8953-mainline/linux v7.0.9-r0), bootloader lk2nd.

> The Wi-Fi-only sibling (SM-T590) is gta2xlwifi / SDA450; this port targets the
> LTE model (SM-T595, SDM450, msm-id 338).

## Layout

```
mainline-port/
├── sdm450-samsung-gta2xllte.dts     # device tree (the source)
├── apply-dts.sh                     # copy dts → kernel tree + verify (out-of-tree)
├── build-bootimg.sh                 # build a RAM-bootable boot.img from the kernel apk
├── upstream/                        # clean BSD-3-Clause dts as submitted upstream
├── pmaports-overlay/                # kernel config changes + diff
├── downstream-dts/                  # downstream Android dts (hardware reference)
├── templates/                       # mainline msm8953 dts (daisy, mido)
└── reference/live-fdt-downstream.dts# decompiled /sys/firmware/fdt
```

## TODO

- Touch: verify/adapt a driver for fts1ba90a.
- DSI panel: ISL98608 i2c config, then `&mdss`/`&mdss_dsi0` instead of simpledrm.
- Package the Wi-Fi NV firmware so it survives a reinstall.
- Modem (LTE), GPU (Adreno 506, `a506_zap`), sensors, audio, charger.
- Spontaneous reboots (likely watchdog).

## Upstream

- Kernel DTS: msm8953-mainline/linux PR #254 (`sdm450-samsung-gta2xllte`).
- lk2nd: gta2xllte already supported upstream (T595*).
