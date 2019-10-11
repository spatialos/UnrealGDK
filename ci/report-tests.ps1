param(
    [string] $test_result_dir
)

$test_array = @()
((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace "<style>", "<script>var scripts = document.getElementsByTagName('script'); for(var i=0;i<scripts.length;i++) {scripts[i].src='ht'+'tps:'+'//'+scripts[i].src;} </script><style>") | Set-Content -Path "$test_result_dir\index.html"
$test_array += @('/bower_components/font-awesome/css/font-awesome.min.css', 'cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css')
# $test_array += @('/bower_components/twentytwenty/css/twentytwenty.css', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/css/twentytwenty.min.css')
# $test_array += @('/bower_components/featherlight/release/featherlight.min.css', 'https://cdnjs.cloudflare.com/ajax/libs/featherlight/1.7.13/featherlight.min.css')
# $test_array += @('/bower_components/bootstrap/dist/css/bootstrap.min.css', 'https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/css/bootstrap.min.css')
# $test_array += @('/bower_components/jquery/dist/jquery.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/jquery/3.1.1/jquery.min.js')
# $test_array += @('/bower_components/jquery.event.move/js/jquery.event.move.js', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/js/jquery.event.move.min.js')
# $test_array += @('/bower_components/jquery_lazyload/jquery.lazyload.js', 'https://cdnjs.cloudflare.com/ajax/libs/jquery_lazyload/1.9.7/jquery.lazyload.min.js')
# $test_array += @('/bower_components/twentytwenty/js/jquery.twentytwenty.js', 'https://cdnjs.cloudflare.com/ajax/libs/mhayes-twentytwenty/1.0.0/js/jquery.twentytwenty.min.js')
# $test_array += @('/bower_components/clipboard/dist/clipboard.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/clipboard.js/1.5.16/clipboard.min.js')
# $test_array += @('/bower_components/anchor-js/anchor.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/anchor-js/3.2.2/anchor.min.js')
# $test_array += @('/bower_components/featherlight/release/featherlight.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/featherlight/1.7.13/featherlight.min.js')
# $test_array += @('/bower_components/bootstrap/dist/js/bootstrap.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/js/bootstrap.min.js')
# $test_array += @('/bower_components/dustjs-linkedin/dist/dust-full.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/dustjs-linkedin/2.7.5/dust-full.min.js')
# $test_array += @('/bower_components/numeral/min/numeral.min.js', 'https://cdnjs.cloudflare.com/ajax/libs/numeral.js/2.0.4/numeral.min.js')

for ($i = 0; $i -lt $test_array.length; $i++) {
    $first = $test_array[$i][0]
    $second = $test_array[$i][1]
    ((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace $first, $second) | Set-Content -Path "$test_result_dir\index.html"
}

((Get-Content -Path "$test_result_dir\index.html" -Raw) -Replace "index.json", "ci%5CTestResults%5Cindex.json") | Set-Content -Path "$test_result_dir\index.html"
