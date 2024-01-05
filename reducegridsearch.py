import tensorflow as tf
from tensorflow.keras import layers, models
from sklearn.model_selection import train_test_split, ParameterGrid
import numpy as np
import matplotlib.pyplot as plt

# Load MNIST dataset
(train_images, _), (test_images, _) = tf.keras.datasets.mnist.load_data()

# Normalize pixel values to be between 0 and 1
train_images, test_images = train_images.astype('float32') / 255.0, test_images.astype('float32') / 255.0

# Split the training set into training and validation sets
train_images, valid_images = train_test_split(train_images, test_size=0.07, random_state=42)

# Define the autoencoder model with batch normalization
def build_autoencoder(activation, optimizer, learning_rate, batch_size, regularization):
    input_img = tf.keras.Input(shape=(28, 28))

    # Encoder
    x = layers.Reshape((28, 28, 1))(input_img)
    x = layers.Conv2D(16, (3, 3), activation=activation, padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Conv2D(8, (3, 3), activation=activation, padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Conv2D(8, (3, 3), activation=activation, padding='same')(x)
    encoded = layers.MaxPooling2D((2, 2), padding='same')(x)

    # Decoder
    x = layers.Conv2D(8, (3, 3), activation=activation, padding='same')(encoded)
    x = layers.UpSampling2D((2, 2))(x)
    x = layers.Conv2D(8, (3, 3), activation=activation, padding='same')(x)
    x = layers.UpSampling2D((2, 2))(x)
    x = layers.Conv2D(16, (3, 3), activation=activation)(x)
    x = layers.UpSampling2D((2, 2))(x)
    decoded = layers.Conv2D(1, (3, 3), activation='sigmoid', padding='same')(x)

    autoencoder = models.Model(input_img, decoded)
    
    # Create an instance of the optimizer
    opt = tf.keras.optimizers.get(optimizer)
    opt.learning_rate = learning_rate

    # Compile the model with the specified optimizer, learning rate, and regularization
    autoencoder.compile(optimizer=opt, loss='binary_crossentropy', metrics=['accuracy', 'mse'])

    return autoencoder

# Define an extended set of hyperparameter options for the grid search
param_grid = {
    'activation': ['relu', 'tanh', 'sigmoid', 'elu'],
    'optimizer': ['adam', 'rmsprop', 'sgd', 'adadelta'],
    'learning_rate': [0.001, 0.01],
    'batch_size': [32, 64, 128],
    'regularization': [None, tf.keras.regularizers.l2(0.001), tf.keras.regularizers.l1(0.001)]
}

# Initialize variables to track the best model and its performance
best_model = None
best_params = None
best_val_loss = float('inf')  # Initialize with a large value

# Generate all possible combinations of hyperparameters
param_combinations = list(ParameterGrid(param_grid))

# Perform the grid search
for params in param_combinations:
    print(f"\nTraining with hyperparameters: {params}")
    
    autoencoder = build_autoencoder(
        activation=params['activation'],
        optimizer=params['optimizer'],
        learning_rate=params['learning_rate'],
        batch_size=params['batch_size'],
        regularization=params['regularization']
    )

    history = autoencoder.fit(
        train_images, train_images,
        epochs=10,
        batch_size=params['batch_size'],
        shuffle=True,
        validation_data=(valid_images, valid_images)
    )

    # Evaluate the model on the validation set
    val_loss = history.history['val_loss'][-1]

    print(f"\nEvaluation results on validation set (loss): {val_loss}")

    # Update the best model if the current model performs better on validation set
    if val_loss < best_val_loss:
        best_val_loss = val_loss
        best_model = autoencoder
        best_params = params

# Print the best hyperparameters
print("\nBest Hyperparameters:")
print(best_params)

# Print or store any other relevant information about the best model
