from flask import Flask, request, render_template_string, send_file
import subprocess
import tempfile
import os
from graphviz import Digraph

app = Flask(__name__)

TEMPLATE = """
<!doctype html>
<html>
<head>
  <title>Vibe Compiler</title>
  <style>
    body { font-family: sans-serif; display: flex; }
    .left, .right { width: 50%; padding: 20px; box-sizing: border-box; }
    textarea { width: 100%; height: 300px; }
    pre { background: #f0f0f0; padding: 10px; }
    img { max-width: 100%; border: 1px solid #ccc; }
  </style>
</head>
<body>
  <div class="left">
    <h1>Vibe Code</h1>
    <form method="post">
      <textarea name="code">{{ code }}</textarea><br>
      <input type="submit" value="Compile">
    </form>

    {% if output %}
    <h2>Output</h2>
    <pre>{{ output }}</pre>
    {% endif %}
  </div>
  <div class="right">
    <h2>Parser Tree (AST)</h2>
    {% if show_ast %}
      <img src="/ast_image">
    {% else %}
      <p>No AST available</p>
    {% endif %}
  </div>
</body>
</html>
"""

@app.route("/", methods=["GET", "POST"])
def index():
    code = ""
    output = ""
    show_ast = False

    if request.method == "POST":
        code = request.form["code"]
        with tempfile.NamedTemporaryFile(mode="w+", suffix=".vibe", delete=False) as f:
            f.write(code)
            temp_file = f.name

        try:
            result = subprocess.run(
                f"./vibe < {temp_file}",
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=os.getcwd()
            )
            output = result.stdout + "\n" + result.stderr
            if "=== AST ===" in output:
                show_ast = True
                generate_ast_image(output)
        finally:
            os.remove(temp_file)

    return render_template_string(TEMPLATE, code=code, output=output, show_ast=show_ast)

@app.route("/ast_image")
def ast_image():
    return send_file("ast.png", mimetype="image/png")

def generate_ast_image(output):
    dot = Digraph(format='png')
    lines = output.splitlines()
    stack = []
    last_id = 0

    def new_id():
        nonlocal last_id
        last_id += 1
        return f"node{last_id}"

    for line in lines:
        if line.strip() == "=== AST ===":
            stack.clear()
            continue
        if line.startswith("==="):
            break
        indent = len(line) - len(line.lstrip())
        label = line.strip()
        node_id = new_id()
        dot.node(node_id, label)
        if stack and indent > stack[-1][0]:
            dot.edge(stack[-1][1], node_id)
        else:
            while stack and indent <= stack[-1][0]:
                stack.pop()
            if stack:
                dot.edge(stack[-1][1], node_id)
        stack.append((indent, node_id))

    dot.render("ast", cleanup=True)

if __name__ == "__main__":
    app.run(debug=True)
