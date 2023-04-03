<!-- ABOUT THE PROJECT -->
## Unreal Engine Plugin for Band

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#dependency">Dependency</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#citation">Citation</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>




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

#### Known issues

Follow description in [Link](https://gist.github.com/dostos/1933decad5261e1d5017e604ae131660) to fix `GoogleVR` runtime error
```
java.lang.NoSuchFieldError: No field ui_settings_button_holder of type I in class Lcom/google/vr/cardboard/R$id; or its superclasses (declaration of ‘com.google.vr.cardboard.R$id’ appears in /data/app/com.abc-1/base.apk) at com.google.vr.cardboard.UiLayer.inflateViewsWithLayoutId(SourceFile:26) at com.google.vr.cardboard.UiLayer.<init>(SourceFile:12)
```

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
3. Replace corresponding library files in `Band\Source\Band\Data\...`

<!-- CONTACT -->
### Contact

Jingyu Lee - jingyu.lee@hcs.snu.ac.kr

### Citation

If you find our work useful, please cite our paper below!

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Tensorflow Lite Support](https://github.com/tensorflow/tflite-support)