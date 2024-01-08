import tensorflow as tf
from tensorflow.keras import layers, models
from sklearn.model_selection import train_test_split
import numpy as np
import matplotlib.pyplot as plt

# Load MNIST dataset
(train_images, _), (test_images, _) = tf.keras.datasets.mnist.load_data()

# Normalize pixel values to be between 0 and 1
train_images, test_images = train_images.astype('float32') / 255.0, test_images.astype('float32') / 255.0

# Split the training set into training and validation sets
train_images, valid_images = train_test_split(train_images, test_size=0.07, random_state=42)

# Define the autoencoder model with batch normalization and leaky relu
import tensorflow as tf
from tensorflow.keras import layers, models

import tensorflow as tf
from tensorflow.keras import layers, models

import tensorflow as tf
from tensorflow.keras import layers, models

import tensorflow as tf
from tensorflow.keras import layers, models

import tensorflow as tf
from tensorflow.keras import layers, models

def build_autoencoder():
    input_img = tf.keras.Input(shape=(28, 28, 1))

    # Encoder
    x = layers.Conv2D(16, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(input_img)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Conv2D(8, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Flatten()(x)
    encoded = layers.Dense(10, activation=layers.LeakyReLU(alpha=0.2))(x)

    # Decoder
    x = layers.Dense(392, activation=layers.LeakyReLU(alpha=0.2))(encoded)
    x = layers.Reshape((7, 7, 8))(x)
    x = layers.Conv2D(8, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.UpSampling2D((2, 2))(x)
    x = layers.Conv2D(16, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.UpSampling2D((2, 2))(x)
    decoded = layers.Conv2D(1, (3, 3), activation='sigmoid', padding='same')(x)

    autoencoder = models.Model(input_img, decoded)
    autoencoder.compile(optimizer='adam', loss='mean_squared_error')

    return autoencoder





# Create the autoencoder model
autoencoder = build_autoencoder()
autoencoder.summary()




autoencoder = build_autoencoder()  
autoencoder.summary()

history = autoencoder.fit(
    train_images, train_images,
    epochs=10,
    batch_size=16,
    shuffle=True,
    validation_data=(valid_images, valid_images)
)

autoencoder.evaluate(test_images, test_images)

encoded_imgs = autoencoder.predict(test_images)

# Plot the training and validation loss
plt.plot(history.history['loss'], label='Training Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.title('Training and Validation Loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.show()

# Plot the original and reconstructed images
n = 10
plt.figure(figsize=(20, 4))
for i in range(n):
    # Display original images
    ax = plt.subplot(2, n, i + 1)
    plt.imshow(test_images[i].reshape(28, 28))
    plt.gray()
    ax.get_xaxis().set_visible(False)
    ax.get_yaxis().set_visible(False)

    # Display reconstructed images
    ax = plt.subplot(2, n, i + 1 + n)
    plt.imshow(encoded_imgs[i].reshape(28, 28))
    plt.gray()
    ax.get_xaxis().set_visible(False)
    ax.get_yaxis().set_visible(False)
plt.show()
