import hidLogo from "../assets/hid_logo.png";
import gallagherLogo from "../assets/gallagher_logo.png";

export default function CardLogo({ cardType }) {
  switch (cardType) {
    case "hid":
      return (
        <img
          className="h-auto max-w-xs mx-auto"
          width="60px"
          src={hidLogo}
          alt="hid-logo"
        />
      );
    case "gallagher":
      return (
        <img
          className="h-auto max-w-xs mx-auto"
          width="90px"
          src={gallagherLogo}
          alt="gallagher-logo"
        />
      );
  }
}
