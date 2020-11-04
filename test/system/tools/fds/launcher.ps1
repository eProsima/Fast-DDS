Param(
    [Parameter(Position=0, Mandatory=$true)]
    [ValidateScript({Test-Path $_ -PathType Leaf -IsValid })]
    [String]
    # python3 binary
    $python_path,

    [Parameter(Position=1, Mandatory=$true)]
    [ValidateScript({Test-Path $_ -PathType Leaf -IsValid })]
    [String]
    # python script that keeps the  testing
    $test_script,

    [Parameter(Position=2, Mandatory=$true)]
    [ValidateScript({Test-Path $_ -PathType Leaf -IsValid })]
    [String]
    # fast server creation binary full qualified path
    $tool_path,

    [Parameter(Position=3, Mandatory=$true)]
    [ValidateNotNullOrEmpty()]
    [String]
    # test_name
    $test_name
)

$test = Start-Process -Passthru -Wait `
    -FilePath $python_path `
    -ArgumentList ($test_script, $tool_path, $test_name) `
    -WindowStyle Hidden

if( $test.ExitCode -ne 0 )
{
    $error_message = "Test: $test_name failed with exit code $($test.ExitCode)."
    throw $error_message
}
