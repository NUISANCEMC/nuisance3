import plotly.graph_objects as go
import plotly.io as pio

def plotly_cern_template():

    # Define a custom template
    cern_root_template = go.layout.Template(
        layout=dict(
            # Set global font
            font=dict(
                family="Arial, sans-serif",
                size=14,
                color="black"
            ),
            
            # Style the plot background and plot area
            plot_bgcolor="white",
            paper_bgcolor="white",
            
            # Customize axis appearance
            xaxis=dict(
                title_font=dict(size=14, family="Arial, sans-serif"),
                title="X",
                showgrid=True,
                gridwidth=1,
                gridcolor="white",
                linecolor="black",
                linewidth=1,
                mirror="allticks",
                showline=True,
                ticks='inside',
                tickfont=dict(size=12, family="Arial, sans-serif"),
            ),
            yaxis=dict(
                title_font=dict(size=14, family="Arial, sans-serif"),
                title="Y",
                showgrid=True,
                gridwidth=1,
                gridcolor="white",
                linecolor="black",
                linewidth=1,
                mirror="allticks",
                showline=True,
                ticks='inside',
                tickfont=dict(size=12, family="Arial, sans-serif"),
            ),
            legend=dict(
                bgcolor="rgba(0,0,0,0)",  # Make legend background transparent
                bordercolor="rgba(0,0,0,0)",  # Make legend border transparent
                orientation="h",  # Make the legend horizontal
                xanchor="center",  # Anchor the legend's x to the center
                yanchor="top",  # Anchor the legend's y to the top
                x=0.5,  # Center the legend horizontally
                y=1.15  # Position the legend just above the plot area
            )
        )
    )

    # Register the template under a custom name
    pio.templates["cern_root"] = cern_root_template

    pio.templates.default = "cern_root"


