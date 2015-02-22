{
  "targets": [
    {
      "target_name": "spi_binding",
      "sources": [ "spi_binding.cc" ],
      "include_dirs" : ["<!(node -e \"require('nan')\")"]
    },
  ]
}
