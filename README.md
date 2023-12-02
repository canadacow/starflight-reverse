# Starflight-Reverse - Remastered #

## What is Starflight and what is this project about? ##

For complete background on this project see the original [here](https://github.com/s-macke/starflight-reverse#readme).

For additional information on Starflight and this interesting time in computer gaming, check out the book [Starflight](https://www.amazon.com/Starflight-Exploded-Computer-Gaming-1987-1994/dp/1732355290) on Amazon.

This specific fork is a **Real-time Rotoscoped Vulkan/AI Generated project bringing Starflight into the 21st century.**

<table border="none">
  <tr>
    <td>
      <img src="https://github.com/canadacow/starflight-reverse/assets/664680/16c4c4c4-754f-495d-bb32-7f4aaf4d0296" width="320" height="240"> 
    </td>
    <td>
      <img src="https://github.com/canadacow/starflight-reverse/assets/664680/89d034cf-9e6e-40e3-95e0-fb6e09a68373" width="320" height="240"> 
    </td>
  </tr>  
</table>

## State of the Build ##

To switch live between the Rotoscoped and classic EGA output, press F1. Once in EGA output, F2 will switch between the EGA and dithered CGA/RGB palette.

The whole game is playable as all of Starflight with both the FORTH code and assembly is fully emulated.

Known issues with emulation:

1. On any communications encounter with aliens, on terminating communications your hull will be destroyed and the health of your ship critical. This appears to be due to corruption happening between overlay loads. This, is in many respects, game breaking and will absolutely need to be addressed independent of rotoscoping.
2. FONT3 doesn't render correctly in emulated modes (haven't found the 9x font table in memory.)
3. Clipped of nebula doesn't work in the Starmap.
4. Technically a 0th issue. As checked in, for expediency and testing purposes, you start with 1,000,000 credits and have infinite money. Fuel is never burned during hyperspace travel.

## Building ##

In theory I have it building on Windows and Linux x64 by way of CMake/Vcpkg. I'm working quickly with to get a whole idea of what needs to be done functionality-wise so I strongly suspect the build is broken for a multitude of reasons at the moment. You will almost certainly have the most luck with Windows x64 and Visual Studio 2022 Community Edition.
