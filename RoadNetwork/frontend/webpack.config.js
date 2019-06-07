var path = require('path');
var fs = require('fs')
var bodyParser = require('body-parser')

module.exports = {
  entry: ["./main.js"], 
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'app.js', 
    publicPath: '/dist/'
  }, 
  module: {
    rules: [
      {
        test: /\.css$/,
        use: [
          { loader: 'style-loader' },
          { loader: 'css-loader' }
        ]
      }, 
      {
        test: /\.js$/,
        exclude: /(node_modules|bower_components)/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: [
              [
                "@babel/preset-env", 
                {
                  "useBuiltIns": "usage",
                  "corejs": 3
                }
              ]
            ]
          }
        }
      }
    ]
  }, 
  devServer: {
    compress: true,
    port: 9000, 
    inline: true, 
    open: true, 
    watchContentBase: true,
    proxy: {
      '/node_file': {
        target: 'http://localhost:1234',
        secure: false
      }, 
      '/car_file': {
        target: 'http://localhost:1234',
        secure: false
      }, 
      '/query_carlist': {
        target: 'http://localhost:1234',
        secure: false
      },
      '/query_car_path': {
        target: 'http://localhost:1234', 
        secure: false
      }
    }
  }
};