
## How to install style

Simply download the zip file from [styles page](http://neuralstyler.com/styles.html) and extract it you will get two files

* style_name.model
* style_name-style.jpg

copy these two files to the styles folder and run NeuralStyler.

## Create your own styles

[Please read the neural network training instructions](https://github.com/yusuketomoto/chainer-fast-neuralstyle#train)

Dependencies

* [Chainer](http://chainer.org/)
* [Microsoft COCO dataset](http://mscoco.org/dataset/#download)

```
python train.py -s (style_image_path) -d (training_dataset_path) -g (use_gpu ? gpu_id : -1)
```