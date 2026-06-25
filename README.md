# postmarketos-samsung-gta2xlwifi — mainline-port

Mainline-portning av Samsung Galaxy Tab A 10.5 (2018, Wi-Fi), "gta2xlwifi",
SoC Qualcomm SDM450 / MSM8953.

## Layout

```
mainline-port/
├── msm8953-samsung-gta2xlwifi.dts   # KÄLLAN — redigera & spara denna
├── apply-dts.sh                     # kopierar dts:en till kärnträdet + verifierar
├── downstream-dts/                  # nedströms Android-dts (referens för hårdvaruvärden)
└── templates/                       # mainline msm8953-dts som mall (daisy, mido)
```

## Arbetsflöde

```bash
cd mainline-port
# 1. redigera & spara msm8953-samsung-gta2xlwifi.dts
./apply-dts.sh                       # 2. applicera i kärnträdet + kompilera dtb (verifiering)
pmbootstrap build --src=/home/simon/pmos/mainline-build/linux-7.0.9-r0 \
    linux-postmarketos-qcom-msm8953  # 3. bygg kärnpaketet
```

Kärnträdet (`~/pmos/mainline-build/linux-7.0.9-r0`, från `msm8953-mainline/linux`
tag v7.0.9-r0) ligger utanför detta repo och är inte versionshanterat här.

## Status

- [x] dts-skelett (modem disablat, touch, knappar, regulatorer, SD/eMMC/USB, konsol)
- [x] dts kompilerar till giltig dtb
- [ ] DSI-panel — körs via lk2nd:s splash + simple-framebuffer (ingen panel-driver krävd)
- [ ] Wi-Fi/BT (&wcnss), sensorer, laddare/fuel-gauge
