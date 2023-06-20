<!-- ABOUT THE PROJECT -->
## Unreal Engine Plugin for Band
### Dependency

* [Band](https://github.com/mrsnu/band)
* [Unreal Engine 4](https://www.unrealengine.com/en-US/)

<!-- GETTING STARTED -->
### Getting Started

#### Prerequisites

1. Unreal Engine version 4.27.2 or above (preferred to build from source)
2. Android Studio version 4.0.0

#### Installation

1. Go to `Plugins` directory of your UE4 project
2. Clone the repo
   ```sh
   git clone https://github.com/mrsnu/ue4-plugin.git
   ```
3. Re-generate visual studio project of root .uproject
4. Enable `Band` plugin from UE editor ([Official Instruction](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/Plugins/))

#### How to update Band Library

1. Clone the repo (currently in `build_script` repo)
   ```sh
   git clone https://github.com/mrsnu/band.git
   ```
2. Build C API from target platform (Windows - Windows, Linux - Android, Linux, ...) 
   ```sh
   python build_c_api.py -windows # windows build for release
   python build_c_api.py -windows -debug # windows build for debug
   python build_c_api.py -android # arm64-v8a build for release
   ...
   ```
3. Replace corresponding library files in `Band\Source\Band\Library\...`

<!-- CONTACT -->
### Contact

Jingyu Lee - jingyu.lee@hcs.snu.ac.kr

### Citation

If you find our work useful, please cite our paper below!

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Tensorflow Lite Support](https://github.com/tensorflow/tflite-support)
