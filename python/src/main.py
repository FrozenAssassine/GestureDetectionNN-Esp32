import os
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim

MODEL_OUTPUT = "out/gesture_model_weights.pth"

# Load data.csv located next to this script
BASE_DIR = os.path.dirname(__file__)
CSV_PATH = os.path.join(BASE_DIR, "data.csv")

data = np.loadtxt(CSV_PATH, delimiter=',', skiprows=1)
# first column is label, remaining columns are features
X_np = data[:, 1:]
y_np = data[:, 0].astype(int) - 1  # make labels 0-based

X = torch.tensor(X_np, dtype=torch.float32)
y = torch.tensor(y_np, dtype=torch.long)

num_classes = int(y.max().item() + 1)
in_features = X.shape[1]


class GestureNet(nn.Module):
    def __init__(self, in_features, num_classes):
        super().__init__()
        self.fc1 = nn.Linear(in_features, 32)
        self.relu = nn.ReLU()
        self.fc2 = nn.Linear(32, num_classes)

    def forward(self, x):
        x = self.relu(self.fc1(x))
        x = self.fc2(x)
        return x


torch.manual_seed(0)
model = GestureNet(in_features, num_classes)

criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=0.01)

epochs = 500
for epoch in range(epochs):
    optimizer.zero_grad()
    logits = model(X)
    loss = criterion(logits, y)
    loss.backward()
    optimizer.step()

    if epoch % 50 == 0:
        print(f"Epoch {epoch}, Loss: {loss.item():.6f}")

with torch.no_grad():
    logits = model(X)
    probs = torch.softmax(logits, dim=1)
    preds = probs.argmax(dim=1)
    print("\nPredictions:")
    for inp, p, pr in zip(X, preds, probs):
        print(
            f"{inp.tolist()} -> label {int(p)+1} prob {pr[p].item():.3f}  probs {[round(float(x), 3) for x in pr.tolist()]}")

os.makedirs(os.path.dirname(MODEL_OUTPUT), exist_ok=True)
torch.save(model.state_dict(), MODEL_OUTPUT)
print(f"Weights saved to {MODEL_OUTPUT}")
