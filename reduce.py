import struct
import tensorflow as tf
from tensorflow.keras import layers, models
from sklearn.model_selection import train_test_split
import numpy as np
import matplotlib.pyplot as plt

(train_images, _), (test_images, _) = tf.keras.datasets.mnist.load_data()

# Normalize pixel values to be between 0 and 1
train_images, test_images = train_images.astype('float32') / 255.0, test_images.astype('float32') / 255.0
train_images_original = train_images
train_images, valid_images = train_test_split(train_images, test_size=0.07, random_state=42)

def build_autoencoder(latent_dimension=10):
    input_img = tf.keras.Input(shape=(28, 28, 1))

    # Encoder
    x = layers.Conv2D(16, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(input_img)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Conv2D(8, (3, 3), activation=layers.LeakyReLU(alpha=0.2), padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Flatten()(x)
    encoded = layers.Dense(latent_dimension, activation=layers.LeakyReLU(alpha=0.2))(x)

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

latent_dim = 10  # You can change this value as needed
autoencoder = build_autoencoder(latent_dimension=latent_dim)
autoencoder.summary()

history = autoencoder.fit(
    train_images, train_images,
    epochs=10,
    batch_size=32,
    shuffle=True,
    validation_data=(valid_images, valid_images)
)

encoder_model = models.Model(inputs=autoencoder.input, outputs=autoencoder.layers[8].output)

encoded_train_imgs = encoder_model.predict(train_images_original)

encoded_test_imgs = encoder_model.predict(test_images)

# Define file paths
train_file_path = 'encoded_train_images.dat' 
test_file_path = 'encoded_test_images.dat'

# Function to save encoded images to a file
def save_encoded_images(file_path, encoded_imgs,latent_dimension=10):
    with open(file_path, 'wb') as file:
        # Write magic number (random 32-bit integer)
        magic_number = np.random.randint(0, 2**31 - 1)
        file.write(struct.pack('>I', magic_number))
        num_images = len(encoded_imgs)
        print ("Number of Images",num_images)
        file.write(struct.pack('>I', num_images))
        file.write(struct.pack('>I', latent_dimension))
        num_columns = 1 #unchangeable
        file.write(struct.pack('>I', num_columns))

        # Iterate through each image
        for img_idx in range(num_images):
            # Write pixel values (unsigned byte)
            for pixel_value in encoded_imgs[img_idx].flatten():
                file.write(struct.pack('B', max(0, min(int(pixel_value * 255), 255))))

save_encoded_images(train_file_path, encoded_train_imgs,latent_dim)
save_encoded_images(test_file_path, encoded_test_imgs,latent_dim)

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
