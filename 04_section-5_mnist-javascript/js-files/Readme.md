# Javascript and JSON files for Duktape and the MNist example

These are multiple file versions that can be useful to test the MNist example. The sgx duktape enclave requires two inputs: The convnet.js Javascript file and a JSON file as input that has to contain the following elements:

```json
{
    "network": ["network here"],
    "input"  : ["mnist input here"]
}
```

The file `trained-network.json` is a valid network that can be used as the content of the first JSON element, while the files `mnist_handwritten_test_first5.json` and `mnist_handwritten_test_first100.json` are valid inputs that can be used as the content of the second JSON element. The files `mnist-network-with-input.json` and `mnist-network-with-input-only-5.json` are simple merges of these files and are ready to use.

| Filename | Description |
| :------- | :---------- |
| convnet.js| The Convnet.JS file developed by the [Convnet.JS project](https://cs.stanford.edu/people/karpathy/convnetjs/), adjusted for usage in Duktape. |
| mnist-network-with-input.json | A merge of `trained-network.json` and `mnist_handwritten_test_first100.json` |
| mnist-network-with-input-only-5.json | A merge of `trained-network.json` and `mnist_handwritten_test_first5.json` |
| mnist_handwritten_test_first100.json | The first 100 images of the MNist test set, compiled as JSON. See the subfolder `extra_mnist-preparation` to see how different MNist images can be used.| 
| mnist_handwritten_test_first5.json | The first 5 images of the MNist test set to have a minimal input that is fast to evaluate.| 
| trained-network.json | One instance of a trained MNist network, exported from the browser. See below how to create your own.|

## Creating a new trained network for the MNist example.

As described in the paper, we based the MNist example on one of the examples provided by Convnet.JS: https://cs.stanford.edu/people/karpathy/convnetjs/demo/mnist.html

To build your own trained network, simply open the website, let the network train for a while and press the "save network snapshot as JSON" button which exports the network to a JSON string that can then be copied to a new file.

In order to use this new file, you can pass it to the run_tests.py script with the -m option. Alternatively, you can assemble your own file by simply combining two JSON files. Essentially, the 