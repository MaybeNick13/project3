import struct

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

# Train the autoencoder
history = autoencoder.fit(
    train_images, train_images,
    epochs=10,
    batch_size=32,
    shuffle=True,
    validation_data=(valid_images, valid_images)
)

# Create a new model to get the encoded features
encoder_model = models.Model(inputs=autoencoder.input, outputs=autoencoder.layers[8].output)

# Save encoded images of the training set
encoded_train_imgs = encoder_model.predict(train_images)

# Save encoded images of the test set
encoded_test_imgs = encoder_model.predict(test_images)

# Define file paths
train_file_path = 'encoded_train_images.dat'
test_file_path = 'encoded_test_images.dat'

# Function to save encoded images to a file
def save_encoded_images(file_path, encoded_imgs):
    with open(file_path, 'wb') as file:
        # Write magic number (random 32-bit integer)
        magic_number = np.random.randint(0, 2**31 - 1)
        file.write(struct.pack('>I', magic_number))

        # Write number of images (32-bit integer)
        num_images = len(encoded_imgs)
        file.write(struct.pack('>I', num_images))

        # Write latent dimension (32-bit integer)
        latent_dimension = 10
        file.write(struct.pack('>I', latent_dimension))

        # Write number of columns (32-bit integer)
        num_columns = 1
        file.write(struct.pack('>I', num_columns))

        # Iterate through each image
        for img_idx in range(num_images):
            # Write pixel values (unsigned byte)
            for pixel_value in encoded_imgs[img_idx].flatten():
                file.write(struct.pack('B', max(0, min(int(pixel_value * 255), 255))))

# Save encoded images of the training set
save_encoded_images(train_file_path, encoded_train_imgs)

# Save encoded images of the test set
save_encoded_images(test_file_path, encoded_test_imgs)

# Print confirmation messages
print(f"Encoded training images saved to {train_file_path}")
print(f"Encoded test images saved to {test_file_path}")

# Plot the training and validation loss
plt.plot(history.history['loss'], label='Training Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.title('Training and Validation Loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.show()
