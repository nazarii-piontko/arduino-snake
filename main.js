var Module = {
    preRun: [],
    postRun: initGame,
    print: function (text) {
        if (arguments.length > 1)
            text = Array.prototype.slice.call(arguments).join(' ');
        console.log(text);
    },
    printErr: function (text) {
        if (arguments.length > 1)
            text = Array.prototype.slice.call(arguments).join(' ');
        console.error(text);
    },
    setStatus: function (text) {
        console.log('WASM status:' + text);
    }
};

function initGame() {
    const canvas = document.getElementById('game');
    const width = ccall('get_canvas_width', 'number', null, null);
    const height = ccall('get_canvas_height', 'number', null, null);

    canvas.width = width;
    canvas.height = height;

    window.addEventListener('keydown', handleKeyDown);

    if (!canvas.getContext) {
        console.error('Unable to get Context from Canvas')
        return;
    }

    const ctx = canvas.getContext('2d');
    const executeGameLoopIteration = cwrap('execute_game_loop_iteration', 'number', null, null)
    const render = cwrap('render', null, null, null)
    const pointer = ccall('init', null, null, null);
    const canvasImageBuffer = new Uint8ClampedArray(Module.HEAPU8.buffer, pointer, 4 * width * height);
    const canvasImage = new ImageData(canvasImageBuffer, width, height);

    function draw() {
        render();
        ctx.putImageData(canvasImage, 0, 0);
    }

    function iterate() {
        const changed = executeGameLoopIteration();
        
        if (changed > 0) {
            window.requestAnimationFrame(draw);
        }

        setTimeout(iterate, 40);
    }

    draw();
    setTimeout(iterate, 40);
}

function handleKeyDown(event) {
    function setInput(input) {
        ccall('set_input', null, ['number'], [input]);
    }

    switch (event.key) {
        case "ArrowLeft":
            setInput(0);
            break;
        case "ArrowRight":
            setInput(1);
            break;
        case "ArrowUp":
            setInput(2);
            break;
        case "ArrowDown":
            setInput(3);
            break;
        case "+":
            ccall('increase_speed', null, null, null);
            break;
        case "-":
            ccall('decrease_speed', null, null, null);
            break;
    }
}