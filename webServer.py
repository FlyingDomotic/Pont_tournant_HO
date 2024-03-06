import os
from aiohttp import web

HOST = os.getenv('HOST', '0.0.0.0')
PORT = int(os.getenv('PORT', 8000))

async def fileHandler(request):
    name = request.match_info.get('name', "index.html")
    if name.lower().endswith(".html"):
        f = open(name,'rt') # open requested file  
        text = f.read()
        f.close()  
        print(f'{name} sent')
        return web.Response(text=text, content_type='text-html')
    elif name.lower().endswith(".png"):
        f = open(name,'rb') # open requested file  
        data = f.read()
        f.close()  
        print(f'{name} sent')
        return web.Response(body=data, content_type='image-png')

def main():
    app = web.Application()
    app.add_routes([
        web.get('/', fileHandler),
        web.get('/{name}', fileHandler)
    ])
    web.run_app(app, host=HOST, port=PORT)

if __name__ == '__main__':
    main()