# Deus Ex 'Echelon Renderer'

Welcome to the Deus Ex Echelon Renderer; a fixed-function rendering pipeline intended for use in with [NVidia's RTX Remix](https://www.nvidia.com/de-de/geforce/rtx-remix/)! 

Note that this is a piece of tech, and not a mod. You can probably play through Deus Ex with this renderer, but you'll want to tweak the default rtx.conf options a bit.
Hopefully, in due time, there will be proper graphics overhaul mods that utilize this or another renderer.

![NvRemixBridge Screenshot 2024 03 29 - 21 06 40 59](https://github.com/onnoj/DeusExEchelonRenderer/assets/4381237/84481569-d305-4110-9f63-6f4a1ee7ee9f)
![NvRemixBridge Screenshot 2024 03 29 - 21 07 18 84](https://github.com/onnoj/DeusExEchelonRenderer/assets/4381237/5b4e6cd6-81ce-4923-9cb4-011c3f561a86)


## How To Use

1. Grab the latest [RTX Remix release](https://github.com/NVIDIAGameWorks/rtx-remix/releases), extract it in Deus Ex's **System** folder.
2. Grab the [latest release of the renderer](https://github.com/onnoj/DeusExEchelonRenderer/releases), extract it in the game's system folder as well.
3. If you haven't already, grab [Deus Exe](https://kentie.net/article/dxguide/), again, Deus Ex's System folder.
4. Start the game, select the "Echelon Renderer" from the list of renderers.
5. If you use Deus Exe:
    * For the best performance, set "FPS Limit" to 0
    * Make sure to turn off "Raw Input", otherwise you cannot interact with RTX Remix: ![image](https://github.com/onnoj/DeusExEchelonRenderer/assets/4381237/60a47f03-eed4-4d02-b1ce-542ac0ee1253)


7. Have fun! (Use alt+x to set a graphics preset)

## For developers: How to build

1. Clone repository recursively (ie with git clone --recurse-submodules)
2. Run cmake on the root folder with 32-bit architecture specified (ie -A win32). For convenience sake, there's a batch file in the BUILD_WIN32 folder.
   You can edit the file to point to your Deus Ex installation folder. 
3. Build
4. Install (use the install target to copy the binaries to the deus ex system folder)

## History

Deus Ex, released in 2000, was built using the Unreal Engine. In terms of rendering architecture, the game's renderer was largely software-driven.
Vertex transformation was done in software, and then pushed to the GPU as a giant vertex soup. The lighting was baked into lightmaps.

Out of the box, Deus Ex (and other Unreal-Engine games) work poorly with Remix; from Remix's perspective, it just sees a giant vertex soup being pushed to the GPU.
That vertex soup is already pre-transformed in view-space, so while some basic raytracing can happen, the results look awful, and things like lighting can never look correct.

Thankfully, the Unreal Engine shipped with a plugin system for its renderers, and Ion-storm were kind enough to release an SDK for Deus Ex which included the headers needed to write new renderers.
Thanks to Marijn Kentie and others, there are various modern open-source renderers available for Deus Ex.

While taking inspiration from those renderers, this renderer is written from scratch with the goal to provide render calls in the most compatible way with RTX Remix.

## Features
- **Real-time lighting**: The renderer uses the 'real' lights from the map data and passes them on to RTX Remix.
- - **Note about Spotlights support**: At the time of writing, spotlights do not update properly (they accumulate over multiple frames) in RTX Remix. This is however working in the latest _development_ builds of RTX Remix, but be wary of dragons.
- **Stable Geometry**: The level geometry is stable, meaning that from RTX Remix's perspective, walking around the world doesn't cause the geometry to change. This greatly reduces flickering. The exception are things that move (like doors).
- **Compatibility hacks**: Deus Ex sometimes has issues running on modern hardware (for example due to clock drift)

## Limitations
- Deus Ex does not have skeletal animation; all animations are baked frame-by-frame vertex soups. From the perspective of RTX Remix, each animation frame is a new mesh. As such, it's probably not possible to replace any animated meshes.
- Level meshes are stable, but object meshes are not, and cannot be replaced efficiently (yet).

## TODO
- **Switch from RH to LH coordinates**: When exporting from the game, the lights have their x coordinates inverted. In-game, this is working correctly. Could be a bug in RTX Remix, so changing coordinate systems might be a fast workaround.
- **Hack out mesh frame clipping**: Currently the meshes are clipped, so characters for example can get clipped a little bit in cutscenes. Additionally, from the perspective of RTX Remix, the clipped mesh becomes unique (and thus cannot be replaced efficiently by the toolkit).
- **Light effects**: Currently, only the 'disco' light effect is implemented. The other effects (such as flickering), should be implemented as well.
- **RTX Hash to Name Mapping**: It could be useful for other tools/scripts to have access to human readable names for textures and what not. Currently I re-calculate the remix texture hash but don't do anything with it.

## TODO TO INVESTIGATE
- See if it would be possible to programatically generate a skeleton, skinning and keyframes from the existing animation frame data. The goal would be to send skinned meshes to RTX Remix instead of per-animation-frame vertex buffers. I expect it would then be possible to replace characters with RTX-Remix.


## Contributing

Contributions to the project are welcome! If you have ideas for improvements or new features, feel free to fork the repository and submit a pull request. Be sure to follow the contribution guidelines outlined in the repository.

## License
Copyright (c) 2024 Onno Jongbloed

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
If you redistribute the renderer or create derivative works based on it, I would appreciate being attributed. Please attribute Onno Jongbloed and provide a link to the original source.

Several external libraries are used, you can find more information about their licenses in the .\EXTERNAL\ directory:
* PolyHook 2.0 - MIT License - Copyright (c) 2018 Stephen Eckels
* Simple DirectMedia Layer - ZLib License - Copyright (C) 1997-2020 Sam Lantinga
* FMT - MIT License - Copyright (c) 2012 - present, Victor Zverovich
* IMGUI  - MIT License - Copyright (c) 2014-2023 Omar Cornut
* xxHash Library - BSD 2-Clause License - Copyright (c) 2012-2021 Yann Collet
* SMHasher - MIT License
* nlohmann/json - MIT License

## Acknowledgements

- Special thanks to Marijn Kentie for the D3D10 renderer, and Chris Dohnal for the D3D9 renderer, both codebases kept the game alive for all these years, and were a tremendous help in the development of this renderer.

## Support

For questions, bug reports, or feature requests, please open an issue on the GitHub repository.
Please bear in mind that this is a hobby project, and I don't have much free time to work on this, the more information (such as screenshots, debugging attempts, etc) you can include, the better.

## Disclaimer

This project is not affiliated with or endorsed by whoever owns the Deus Ex IP/rights or what became of Ion Storm, Eidos Interactive, or any associated entities. 
It is an independent, fan-made project created for educational and recreational purposes.
