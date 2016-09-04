# NeuralStyler
Turn Your Videos/photos into Artwork

NeuralStyler Artificial Intelligence converts your videos into art works by using styles of famous artists: Van Gogh,Wassily Kandinsky,Georges Seurat etc
Features
--------
* Style videos,gif animation and photos
* No need to upload videos (Offline processing)
* Faster AI styling algorithm
* Extensible styling system(Plugin)

###Dependencies
* Qt 5.x
* Python 2.7 (Virtualenv for Ubuntu,WinPython 64 bit for Windows)
* Chainer
* ffmpeg

###How to install style</h3>
Simply download the zip file and extract it you will get two files<br>
	 **style_name.model** 
	 **style_name-style.jpg**
	 copy these two files to the styles folder and run NeuralStyler.

###Create your own styles
Please read the neural network training instructions
[Read](https://github.com/yusuketomoto/chainer-fast-neuralstyle#train)

####Dependencies
[Chainer](http://chainer.org) <br>
[Microsoft COCO dataset](http://mscoco.org/dataset/#download)

python train.py -s (style_image_path) -d (training_dataset_path) -g (use_gpu ? gpu_id : -1)
